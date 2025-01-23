#include "message.h"
#include <store/context.h>
#include <store/utils.h>
#include <plexus/plexus.h>
#include <wormhole/logger.h>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/process/v2/process.hpp>
#include <boost/process/v2/environment.hpp>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <future>
#include <regex>
#include <map>
#include <set>

#ifdef WIN32
    #include <windows.h>
#endif

namespace slipway
{
    namespace 
    {
        constexpr char stun_server_default_port[] = "3478";
        constexpr char smtp_server_default_port[] = "smtps";
        constexpr char imap_server_default_port[] = "imaps";
        constexpr const char* webpier_conf_file_name = "webpier.json";
        constexpr const char* webpier_lock_file_name = "webpier.lock";
        constexpr const char* slipway_lock_file_name = "slipway.lock";
        constexpr const char* slipway_jack_file_name = "slipway.jack";

        boost::posix_time::seconds get_retry_timeout()
        {
            const char* timeout = std::getenv("WEBPIER_RETRY_TIMEOUT");
            try
            {
                return boost::posix_time::seconds(timeout ? std::stoi(timeout) : 20);
            }
            catch (const std::exception& ex)
            {
                _err_ << "can't parse retry timeout: " << ex.what();
            }

            return boost::posix_time::seconds(20);
        }

        template<class protocol>
        typename protocol::endpoint resolve(const std::string& url, const std::string& service)
        {
            if (url.empty() && service.empty())
                return typename protocol::endpoint();

            boost::asio::io_context io;
            typename protocol::resolver resolver(io);

            std::smatch match;
            if (std::regex_search(url, match, std::regex("^(\\w+://)?\\[([a-zA-Z0-9:]+)\\]:(\\d+).*")))
                return *resolver.resolve(match[2].str(), match[3].str());

            if (std::regex_search(url, match, std::regex("^(\\w+)://\\[([a-zA-Z0-9:]+)\\].*")))
                return *resolver.resolve(match[2].str(), match[1].str());

            if (std::regex_search(url, match, std::regex("^\\[([a-zA-Z0-9:]+)\\].*")))
                return *resolver.resolve(match[1].str(), service);

            if (std::regex_search(url, match, std::regex("^(\\w+://)?([\\w\\.]+):(\\d+).*")))
                return *resolver.resolve(match[2].str(), match[3].str());

            if (std::regex_search(url, match, std::regex("^(\\w+)://([\\w\\.]+).*")))
                return *resolver.resolve(match[2].str(), match[1].str());

            return *resolver.resolve(url, service);
        }

        plexus::options make_options(const webpier::config& config, const webpier::service& service)
        {
            return plexus::options {
                service.name,
                config.repo,
                resolve<boost::asio::ip::udp>(config.nat.stun, stun_server_default_port),
                {},
                config.nat.hops,
                service.rendezvous.empty()
                    ? plexus::rendezvous {
                        plexus::emailer {
                            resolve<boost::asio::ip::tcp>(config.email.smtp, smtp_server_default_port),
                            resolve<boost::asio::ip::tcp>(config.email.imap, imap_server_default_port),
                            config.email.login,
                            config.email.password,
                            config.email.cert,
                            config.email.key,
                            config.email.ca 
                        }}
                    : plexus::rendezvous {
                        plexus::dhtnode {
                            service.rendezvous,
                            config.dht.port,
                            config.dht.network
                        }}
                };
        }

        plexus::identity make_identity(const std::string& pier)
        {
            static constexpr const char SLASH = '/';
            size_t pos = pier.find(SLASH);

            return pos != std::string::npos
                ? plexus::identity { pier.substr(0, pos), pier.substr(pos + 1) } 
                : plexus::identity { pier, "" };
        }

        std::string stringify(const boost::asio::ip::udp::endpoint& ep)
        {
            return ep.address().to_string() + ":" + std::to_string(ep.port());
        }

        std::string make_log_path(const std::string& folder)
        {
            if (folder.empty())
                return "";

            std::time_t time = std::time(0);
            std::tm tm = *std::localtime(&time);
            std::stringstream ss;
            ss << folder << std::put_time(&tm, "/slipway.%Y%m%d.log");

            return ss.str();
        }

        class spawner
        {
            class session
            {
                health::status m_state;
                boost::asio::io_context m_io;
                std::future<bool> m_job;

                void start()
                {
                    if (!m_job.valid())
                    {
                        m_state = health::active;
                        m_job = std::async(std::launch::async, [=]()
                        {
                            bool ok = true;
                            try
                            {
                                m_io.run();
                            }
                            catch(const std::exception& ex)
                            {
                                ok = false;
                                _err_ << "session failed: " << ex.what();
                            }
                            return ok;
                        });
                    }
                }

                void stop()
                {
                    if (m_job.valid())
                    {
                        m_io.stop();
                        m_job.wait();
                        m_state = health::status::asleep;
                    }
                }

            public:

                session() : m_state(health::status::asleep)
                {
                }

                ~session()
                {
                    stop();
                }

                void spawn_invite(const plexus::options& options, const plexus::identity& host, const plexus::identity& peer, const plexus::connector& connect, const plexus::fallback& fallback)
                {
                    plexus::spawn_invite(m_io, options, host, peer, connect, fallback);
                    start();
                }

                void spawn_accept(const plexus::options& options, const plexus::identity& host, const plexus::identity& peer, const plexus::connector& connect, const plexus::fallback& fallback)
                {
                    plexus::spawn_accept(m_io, options, host, peer, connect, fallback);
                    start();
                }

                health::status state()
                {
                    if (m_job.valid() && m_job.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
                    {
                        if (!m_job.get())
                            m_state = health::failed;
                    }
                    return m_state;
                }
            };

            void start_export(const webpier::config& conf, const webpier::service& serv)
            {
                using namespace plexus;

                m_work.reset(new session());

                auto opts = make_options(conf, serv);
                auto host = make_identity(conf.pier);

                std::set<std::string> piers;
                boost::split(piers, serv.pier, boost::is_any_of(" "));

                for(auto& item : m_pool)
                {
                    if (piers.find(item.first) == piers.end())
                        item.second.terminate();
                }

                auto on_accept = [this, conf, serv](const identity&, const identity& peer, const udp::endpoint& bind, const reference& self, const reference& mate)
                {
                    m_io.post([this, conf, serv, bind, self, peer, mate]()
                    {
                        boost::process::v2::process proc(m_io, webpier::find_exec("WORMHOLE_EXEC", WORMHOLE_EXEC), 
                        {
                            "--purpose=export", 
                            "--service=" + serv.address,
                            "--gateway=" + stringify(bind),
                            "--faraway=" + stringify(mate.endpoint),
                            "--obscure=" + std::to_string(serv.obscure ? self.puzzle ^ mate.puzzle : 0),
                            "--log-file=" + (conf.log.folder.empty() ? "" : conf.log.folder + "/export.%p.log"),
                            "--log-level=" + std::to_string(conf.log.level)
                        });

                        _inf_ << "spawned process " << proc.id() << " to export " << serv.name << " to " << peer.owner << "/" << peer.pin;

                        proc.async_wait([this, serv, id = proc.id()](const boost::system::error_code& ec, int code)
                        {
                            _inf_ << "joined process " << id << " with status " << code << " " << ec.message();

                            auto range = m_pool.equal_range(serv.pier);
                            m_pool.erase(std::find_if(range.first, range.second, [id](const auto& item)
                            {
                                return item.second.id() == id;
                            }));
                        });

                        m_pool.emplace(serv.pier, std::move(proc));
                    });
                };

                auto on_error = [serv](const identity&, const identity& peer, const std::string& error)
                {
                    _err_ << "export " << serv.name << " to " << peer.owner << "/" << peer.pin << " failed: " << error;
                };

                for (const auto& pier : piers)
                {
                    _dbg_ << "start export " << serv.name << " to " << pier;
                    m_work->spawn_accept(opts, host, make_identity(pier), on_accept, on_error);
                }
            }

            void start_import(const webpier::config& conf, const webpier::service& serv)
            {
                using namespace plexus;

                m_work.reset(new session());

                boost::system::error_code ec;
                m_timer.cancel(ec);

                for(auto& item : m_pool)
                {
                    item.second.terminate();
                }

                auto opts = make_options(conf, serv);
                auto host = make_identity(conf.pier);
                auto peer = make_identity(serv.pier);

                auto on_invite = [this, conf, serv](const identity&, const identity&, const udp::endpoint& bind, const reference& self, const reference& mate)
                {
                    m_io.post([this, conf, serv, bind, self, mate]()
                    {
                        boost::process::v2::process proc(m_io, webpier::find_exec("WORMHOLE_EXEC", WORMHOLE_EXEC), 
                        {
                            "--purpose=import", 
                            "--service=" + serv.address,
                            "--gateway=" + stringify(bind),
                            "--faraway=" + stringify(mate.endpoint),
                            "--obscure=" + std::to_string(serv.obscure ? self.puzzle ^ mate.puzzle : 0),
                            "--log-file=" + (conf.log.folder.empty() ? "" : conf.log.folder + "/import.%p.log"),
                            "--log-level=" + std::to_string(conf.log.level)
                        });

                        _inf_ << "spawned process " << proc.id() << " to import " << serv.pier << " from " << serv.name;

                        proc.async_wait([this, conf, serv, id = proc.id()](const boost::system::error_code& ec, int code)
                        {
                            _inf_ << "joined process " << id << " with status " << code << " " << ec.message();

                            auto range = m_pool.equal_range(serv.pier);
                            m_pool.erase(std::find_if(range.first, range.second, [id](const auto& item)
                            {
                                return item.second.id() == id;
                            }));

                            m_timer.expires_from_now(get_retry_timeout());
                            m_timer.async_wait([this, conf, serv](const boost::system::error_code& ec)
                            {
                                if (ec)
                                    return;

                                start_import(conf, serv);
                            });
                        });

                        m_pool.emplace(serv.pier, std::move(proc));
                    });
                };

                auto on_error = [this, conf, serv](const identity&, const identity&, const std::string& error)
                {
                    _err_ << "import " << serv.name << " from " << serv.pier << " failed: " << error;

                    m_io.post([this, conf, serv]()
                    {
                        m_timer.expires_from_now(get_retry_timeout());
                        m_timer.async_wait([this, conf, serv](const boost::system::error_code& ec)
                        {
                            if (ec)
                                return;

                            start_import(conf, serv);
                        });
                    });
                };

                _dbg_ << "start import " << serv.name << " from " << serv.pier;
                m_work->spawn_invite(opts, host, peer, on_invite, on_error);
            }

        public:

            spawner(boost::asio::io_context& io) 
                : m_io(io)
                , m_timer(m_io)
            {
            }

            spawner(spawner&& other) 
                : m_io(other.m_io)
                , m_timer(std::move(other.m_timer))
                , m_work(std::move(other.m_work))
                , m_pool(std::move(other.m_pool))
            {
            }

            void suspend()
            {
                m_work.reset();
                
                boost::system::error_code ec;
                m_timer.cancel(ec);

                for(auto& item : m_pool)
                    item.second.terminate();
            }

            void restore(const std::string& pier, const webpier::config& conf, const webpier::service& serv)
            {
                conf.pier == pier
                    ? start_export(conf, serv) 
                    : start_import(conf, serv);
            }

            health::status state()
            {
                return m_work ? m_work->state() : health::asleep;
            }

            std::vector<report::spawn> asset()
            {
                std::vector<report::spawn> res;

                for(auto& item : m_pool)
                    res.emplace_back(report::spawn{ item.first, static_cast<uint32_t>(item.second.id()) });

                return std::vector<report::spawn>(std::move(res));
            }

        private:

            boost::asio::io_context& m_io;
            boost::asio::deadline_timer m_timer;
            std::shared_ptr<session> m_work;
            std::multimap<std::string, boost::process::v2::process> m_pool;
        };

        class engine
        {
            boost::asio::io_context& m_io;
            std::map<handle, spawner> m_pool;
            std::filesystem::path m_home;

            webpier::config load_config() noexcept(false)
            {
                auto file = m_home / webpier_conf_file_name;
                webpier::config conf;

                boost::property_tree::ptree doc;
                boost::property_tree::read_json(file.string(), doc);

                return webpier::config {
                    doc.get<std::string>("pier"),
                    doc.get<std::string>("repo"),
                    webpier::journal {
                        doc.get<std::string>("log.folder", ""),
                        webpier::journal::severity(doc.get<int>("log.level", webpier::journal::info))
                    },
                    webpier::puncher {
                        doc.get<std::string>("nat.stun"),
                        doc.get<uint8_t>("nat.hops", 7)
                    },
                    webpier::dhtnode {
                        doc.get<std::string>("dht.bootstrap", ""),
                        doc.get<uint16_t>("dht.port", 0),
                        0
                    },
                    webpier::emailer {
                        doc.get<std::string>("email.smtp", ""),
                        doc.get<std::string>("email.imap", ""),
                        doc.get<std::string>("email.login", ""),
                        doc.get<std::string>("email.password", ""),
                        doc.get<std::string>("email.cert", ""),
                        doc.get<std::string>("email.key", ""),
                        doc.get<std::string>("email.ca", "")
                    },
                    doc.get<bool>("autostart")
                };
            }

            webpier::service load_config(const std::filesystem::path& file, const std::string& service) noexcept(false)
            {
                if (std::filesystem::exists(file))
                {
                    boost::property_tree::ptree doc;
                    boost::property_tree::read_json(file.string(), doc);

                    boost::property_tree::ptree array;
                    for (auto& item : doc.get_child("services", array))
                    {
                        if (item.second.get<std::string>("name") == service)
                        {
                            return webpier::service {
                                item.second.get<std::string>("name"),
                                item.second.get<std::string>("pier"),
                                item.second.get<std::string>("address"),
                                item.second.get<std::string>("rendezvous", ""),
                                item.second.get<bool>("autostart", false),
                                item.second.get<bool>("obscure", true),
                            };
                        }
                    }
                }

                throw std::runtime_error("unknown service");
            }

            std::vector<webpier::service> load_config(const std::filesystem::path& file) noexcept(false)
            {
                std::vector<webpier::service> res;

                if (std::filesystem::exists(file))
                {
                    boost::property_tree::ptree doc;
                    boost::property_tree::read_json(file.string(), doc);

                    boost::property_tree::ptree array;
                    for (auto& item : doc.get_child("services", array))
                    {
                        res.emplace_back(webpier::service {
                            item.second.get<std::string>("name"),
                            item.second.get<std::string>("pier"),
                            item.second.get<std::string>("address"),
                            item.second.get<std::string>("rendezvous", ""),
                            item.second.get<bool>("autostart", false),
                            item.second.get<bool>("obscure", true)
                        });
                    }
                }

                return std::vector<webpier::service>(std::move(res));
            }

            void adjust() noexcept(false)
            {
                auto locker = m_home / webpier_lock_file_name;
                boost::interprocess::file_lock guard(locker.string().c_str());
                boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(guard);

                webpier::config conf = load_config();

                wormhole::log::set(wormhole::log::severity(conf.log.level), make_log_path(conf.log.folder));

                _inf_ << "adjust...";

                std::map<handle, spawner> pool;
                for (auto const& owner : std::filesystem::directory_iterator(conf.repo))
                {
                    if (!owner.is_directory())
                        continue;

                    for (auto const& pin : std::filesystem::directory_iterator(owner.path()))
                    {
                        if (!pin.is_directory())
                            continue;

                        auto file = pin.path() / webpier_conf_file_name;
                        if (!std::filesystem::exists(file))
                            continue;

                        auto pier = owner.path().filename().string() + "/" + pin.path().filename().string();
                        for (const auto& serv : load_config(file))
                        {
                            handle id { pier, serv.name };

                            auto iter = m_pool.find(id);
                            if (iter != m_pool.end())
                                iter = pool.emplace(id, std::move(iter->second)).first;
                            else
                                iter = pool.emplace(id, spawner(m_io)).first;

                            if (serv.autostart)
                            {
                                _inf_ << "restore " << pier << ":" << serv.name;
                                iter->second.restore(pier, conf, serv);
                            } 
                            else if (iter->second.state() != slipway::health::asleep)
                            {
                                _inf_ << "suspend " << pier << ":" << serv.name;
                                iter->second.suspend();
                            }
                        }
                    }
                }

                std::swap(m_pool, pool);
            }

            void engage(const slipway::handle& id) noexcept(false)
            {
                auto locker = m_home / webpier_lock_file_name;
                boost::interprocess::file_lock guard(locker.string().c_str());
                boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(guard);

                webpier::config conf = load_config();
                webpier::service serv = load_config(std::filesystem::path(conf.repo) / id.pier / webpier_conf_file_name, id.service);

                auto iter = m_pool.find(id);
                if (iter == m_pool.end())
                    iter = m_pool.emplace(id, spawner(m_io)).first;

                _inf_ << "restore " << id.pier << ":" << id.service;

                iter->second.restore(id.pier, conf, serv);
            }

            void unplug() noexcept(false)
            {
                _inf_ << "unplug...";

                for (auto& item : m_pool)
                {
                    if (item.second.state() != slipway::health::asleep)
                    {
                        _inf_ << "suspend " << item.first.pier << ":" <<  item.first.service;
                        item.second.suspend();
                    }
                }
            }

            void unplug(const slipway::handle& id) noexcept(false)
            {
                auto iter = m_pool.find(id);
                if (iter != m_pool.end())
                {
                    if (iter->second.state() != slipway::health::asleep)
                    {
                        _inf_ << "suspend " << id.pier << ":" << id.service;
                        iter->second.suspend();
                    }
                }
            }

            std::vector<slipway::health> status() noexcept(false)
            {
                std::vector<slipway::health> res;
                for (auto& item : m_pool)
                    res.emplace_back(report::health{ item.first, item.second.state() });
                return res;
            }

            slipway::health status(const slipway::handle& id) noexcept(false)
            {
                auto iter = m_pool.find(id);
                if (iter != m_pool.end())
                    return slipway::health{ id, iter->second.state() };

                throw std::runtime_error("unknown service");
            }

            std::vector<slipway::report> report() noexcept(false)
            {
                std::vector<slipway::report> res;
                for (auto& item : m_pool)
                    res.emplace_back(slipway::report{ slipway::health{ item.first, item.second.state() }, item.second.asset() });
                return res;
            }

            slipway::report report(const slipway::handle& id) noexcept(false)
            {
                auto iter = m_pool.find(id);
                if (iter != m_pool.end())
                    return slipway::report { slipway::health { iter->first, iter->second.state() }, iter->second.asset() };

                throw std::runtime_error("unknown service");
            }

        public:

            engine(boost::asio::io_context& io, const std::filesystem::path& home)
                : m_io(io)
                , m_home(home)
            {
                adjust();
            }

            void handle_request(boost::asio::streambuf& buffer) noexcept(true)
            {
                slipway::message req, res;

                try
                {
                    slipway::pull_message(buffer, req);

                    _trc_ << "handle request: action=" << req.action << " payload=" << req.payload.index();

                    switch (req.action)
                    {
                        case slipway::message::unplug:
                        {
                            req.payload.index() == 1 
                                ? unplug(std::get<slipway::handle>(req.payload)) 
                                : unplug();
                            res = slipway::message::make(slipway::message::unplug);
                            break;
                        }
                        case slipway::message::adjust:
                        {
                            adjust();
                            res = slipway::message::make(slipway::message::adjust);
                            break;
                        }
                        case slipway::message::engage:
                        {
                            engage(std::get<slipway::handle>(req.payload));
                            res = slipway::message::make(slipway::message::engage);
                            break;
                        }
                        case slipway::message::status:
                        {
                            res = req.payload.index() == 1
                                ? slipway::message::make(slipway::message::status, status(std::get<slipway::handle>(req.payload)))
                                : slipway::message::make(slipway::message::status, status());
                            break;
                        }
                        case slipway::message::review:
                        {
                            res = req.payload.index() == 1
                                ? slipway::message::make(slipway::message::review, report(std::get<slipway::handle>(req.payload)))
                                : slipway::message::make(slipway::message::review, report());
                            break;
                        }
                        default:
                            res = slipway::message::make(req.action, "wrong command");
                            break;
                    }
                }
                catch (const std::exception& ex)
                {
                    _err_ << "can't handle request: error=" << ex.what();
                    res = slipway::message::make(req.action, ex.what());
                }

                slipway::push_message(buffer, res);
            }
        };

        class server
        {
            boost::asio::io_context& m_io;
            boost::asio::local::stream_protocol::acceptor m_acceptor;
            slipway::engine m_engine;
            size_t m_score;

            void handle(boost::asio::local::stream_protocol::socket client)
            {
                boost::asio::spawn(m_io, [this, socket = std::move(client)](boost::asio::yield_context yield) mutable
                {
                    boost::asio::streambuf buffer;
                    boost::system::error_code ec;

                    auto cleanup = [&]()
                    {
                        if (ec != boost::asio::error::operation_aborted)
                            _wrn_ << ec.message();

                        if (--m_score == 0)
                            m_io.stop();

                        ec = socket.close(ec);
                    };

                    boost::asio::async_read_until(socket, buffer, '\n', yield[ec]);
                    if (ec)
                        return cleanup();

                    m_engine.handle_request(buffer);

                    boost::asio::async_write(socket, buffer, yield[ec]);
                    if (ec)
                        return cleanup();

                    handle(std::move(socket));
                });
            }

            void accept()
            {
                m_acceptor.async_accept([this](boost::system::error_code ec, boost::asio::local::stream_protocol::socket socket)
                {
                    if (ec)
                    {
                        if (ec != boost::asio::error::operation_aborted)
                            throw boost::system::system_error(ec, ec.message());

                        m_io.stop();
                        return;
                    }

                    ++m_score;

                    _trc_ << "accepted client";

                    handle(std::move(socket));
                    accept();
                });
            }

        public:

            server(boost::asio::io_context& io, const boost::asio::local::stream_protocol::endpoint& ep, const std::filesystem::path& home, bool steady)
                : m_io(io)
                , m_acceptor(io, ep.protocol())
                , m_engine(io, home)
                , m_score(steady ? 1 : 0)
            {
                m_acceptor.bind(ep);
                m_acceptor.listen();

                accept();
            }
        };
    }
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "no home argument" << std::endl;
            return 1;
        }

        std::filesystem::path home = std::filesystem::path(argv[1]);
        if (!std::filesystem::exists(home) || !std::filesystem::exists(home / slipway::webpier_conf_file_name))
        {
            std::cerr << "wrong home argument" << std::endl;
            return 2;
        }

        std::filesystem::path socket = home / slipway::slipway_jack_file_name;
        std::filesystem::path locker = home / slipway::slipway_lock_file_name;

        if (!std::filesystem::exists(locker))
            std::ofstream(locker).close();

        boost::interprocess::file_lock guard(locker.string().c_str());
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(guard, boost::interprocess::try_to_lock_type());

        if (!lock.owns())
        {
            std::cerr << "can't acquire lock" << std::endl;
            return 3;
        }

#ifdef WIN32
        DeleteFile(socket.string().c_str());
#else
        ::unlink(socket.string().c_str());
#endif

        boost::asio::io_context io;
        slipway::server server(io, socket.string(), home, argc == 3 && std::strcmp(argv[2], "daemon") == 0);
        io.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 4;
    }

    return 0;
}
