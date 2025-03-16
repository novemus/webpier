#include <backend/message.h>
#include <memory>
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
        constexpr const int default_retry_timeout = 15;
        constexpr const char* stun_server_default_port = "3478";
        constexpr const char* stun_client_default_port = "0";
        constexpr const char* smtp_server_default_port = "smtps";
        constexpr const char* imap_server_default_port = "imaps";
        constexpr const char* webpier_conf_file_name = "webpier.json";
        constexpr const char* webpier_lock_file_name = "webpier.lock";
        constexpr const char* slipway_lock_file_name = "slipway.lock";
        constexpr const char* slipway_jack_file_name = "slipway.jack";

        boost::posix_time::seconds get_retry_timeout()
        {
            const char* timeout = std::getenv("WEBPIER_RETRY_TIMEOUT");
            try
            {
                return boost::posix_time::seconds(timeout ? std::stoi(timeout) : default_retry_timeout);
            }
            catch (const std::exception& ex)
            {
                _err_ << "can't parse retry timeout: " << ex.what();
            }

            return boost::posix_time::seconds(default_retry_timeout);
        }

        template<class protocol>
        typename protocol::endpoint resolve(const std::string& url, const std::string& service)
        {
            try 
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
            catch (const std::exception& ex)
            {
                _err_ << ex.what();
                throw std::runtime_error("can't resolve " + url);
            }
        }

        plexus::options make_options(const webpier::config& config, const webpier::service& service)
        {
            return plexus::options {
                service.name,
                config.repo,
                resolve<boost::asio::ip::udp>(config.nat.stun, stun_server_default_port),
                resolve<boost::asio::ip::udp>(service.gateway, stun_client_default_port),
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

        std::string stringify(const plexus::identity& id)
        {
            return id.owner + "/" + id.pin;
        }

        std::string stringify(const boost::asio::ip::udp::endpoint& ep)
        {
            return ep.address().to_string() + ":" + std::to_string(ep.port());
        }

        std::string make_log_path(const std::string& folder)
        {
            if (folder.empty())
                return "";

            return folder + webpier::make_timestamp("/slipway.%Y%m%d.log");
        }

        class controller : public std::enable_shared_from_this<controller>
        {
            struct session
            {
                boost::asio::io_context io;
                std::shared_future<std::string> future;
            };

            boost::asio::io_context& m_io;
            boost::asio::deadline_timer m_timer;
            std::shared_ptr<session> m_job;
            std::multimap<std::string, boost::process::v2::process> m_pool;

            void start(const std::function<void()>& task)
            {
                m_job = std::make_shared<session>();

                m_job->io.post(task);
                m_job->future = std::async(std::launch::async, [this, task]
                {
                    std::string error;
                    try
                    {
                        m_job->io.run();
                    }
                    catch(const std::exception& ex)
                    {
                        std::weak_ptr<controller> weak = shared_from_this();

                        m_timer.expires_from_now(get_retry_timeout());
                        m_timer.async_wait([weak, task](const boost::system::error_code& ec)
                        {
                            if (ec)
                                return;

                            auto ptr = weak.lock();
                            if(ptr && ptr->state() != health::asleep)
                                ptr->start(task);
                        });

                        _err_ << ex.what();
                        error = ex.what();
                    }

                    return error;
                });
            }

            void stop()
            {
                if (m_job && m_job->future.valid())
                {
                    m_job->io.stop();
                    m_job->future.wait();
                }

                boost::system::error_code ec;
                m_timer.cancel(ec);

                m_job.reset();
            }

            void start_export(const webpier::config& conf, const webpier::service& serv)
            {
                std::set<std::string> piers;
                boost::split(piers, serv.pier, boost::is_any_of(" "));

                for(auto& item : m_pool)
                {
                    if (piers.find(item.first) == piers.end())
                        item.second.terminate();
                }

                start([this, conf, serv, piers]()
                {
                    using namespace plexus;

                    auto opts = make_options(conf, serv);
                    auto host = make_identity(conf.pier);
    
                    auto on_accept = [this, conf, serv](const identity&, const identity& peer, const udp::endpoint& bind, const reference& self, const reference& mate)
                    {
                        boost::process::v2::process proc(m_io, webpier::get_module_path(CARRIER_MODULE).string(), 
                        {
                            "--purpose=export", 
                            "--service=" + serv.address,
                            "--gateway=" + stringify(bind),
                            "--faraway=" + stringify(mate.endpoint),
                            "--obscure=" + std::to_string(serv.obscure ? self.puzzle ^ mate.puzzle : 0),
                            "--journal=" + (conf.log.folder.empty() ? "" : conf.log.folder + "/export.%p.log"),
                            "--logging=" + std::to_string(conf.log.level)
                        });

                        _inf_ << "spawned process " << proc.id() << " to export " << serv.name << " to " << stringify(peer);

                        std::weak_ptr<controller> weak = shared_from_this();
                        proc.async_wait([this, weak, serv, peer, id = proc.id()](const boost::system::error_code& ec, int code)
                        {
                            _inf_ << "joined process " << id << " with exit code " << code << " (" << ec.message() << ")";

                            if (auto ptr = weak.lock())
                            {
                                auto range = m_pool.equal_range(stringify(peer));
                                m_pool.erase(std::find_if(range.first, range.second, [id](const auto& item)
                                {
                                    return item.second.id() == id;
                                }));
                            }
                        });

                        m_pool.emplace(stringify(peer), std::move(proc));
                    };

                    auto on_error = [serv](const identity&, const identity& peer, const std::string& error)
                    {
                        _err_ << "export " << serv.name << " to " << stringify(peer) << " failed: " << error;
                    };
    
                    for (const auto& pier : piers)
                    {
                        _dbg_ << "start export " << serv.name << " to " << pier;
                        plexus::spawn_accept(m_job->io, opts, host, make_identity(pier), on_accept, on_error);
                    }
                });
            }

            void start_import(const webpier::config& conf, const webpier::service& serv)
            {
                for(auto& item : m_pool)
                    item.second.terminate();

                start([this, conf, serv]()
                {
                    using namespace plexus;
    
                    auto opts = make_options(conf, serv);
                    auto host = make_identity(conf.pier);
                    auto peer = make_identity(serv.pier);
    
                    auto on_invite = [this, conf, serv](const identity&, const identity&, const udp::endpoint& bind, const reference& self, const reference& mate)
                    {
                        boost::process::v2::process proc(m_io, webpier::get_module_path(CARRIER_MODULE).string(), 
                        {
                            "--purpose=import", 
                            "--service=" + serv.address,
                            "--gateway=" + stringify(bind),
                            "--faraway=" + stringify(mate.endpoint),
                            "--obscure=" + std::to_string(serv.obscure ? self.puzzle ^ mate.puzzle : 0),
                            "--journal=" + (conf.log.folder.empty() ? "" : conf.log.folder + "/import.%p.log"),
                            "--logging=" + std::to_string(conf.log.level)
                        });

                        _inf_ << "spawned process " << proc.id() << " to import " << serv.pier << " from " << serv.name;

                        std::weak_ptr<controller> weak = shared_from_this();
                        proc.async_wait([this, weak, conf, serv, id = proc.id()](const boost::system::error_code& ec, int code)
                        {
                            _inf_ << "joined process " << id << " with exit code " << code << " (" << ec.message() << ")";

                            if (auto ptr = weak.lock())
                            {
                                auto range = m_pool.equal_range(serv.pier);
                                m_pool.erase(std::find_if(range.first, range.second, [id](const auto& item)
                                {
                                    return item.second.id() == id;
                                }));

                                if (ptr->state() != health::asleep)
                                {
                                    m_timer.expires_from_now(get_retry_timeout());
                                    m_timer.async_wait([weak, conf, serv](const boost::system::error_code& ec)
                                    {
                                        if (ec)
                                            return;
            
                                        auto ptr = weak.lock();
                                        if(ptr && ptr->state() != health::asleep)
                                            ptr->start_import(conf, serv);
                                    });
                                }
                            }
                        });

                        m_pool.emplace(serv.pier, std::move(proc));
                    };
    
                    auto on_error = [this, serv](const identity&, const identity&, const std::string& error)
                    {
                        _err_ << "import " << serv.name << " from " << serv.pier << " failed: " << error;
                        throw std::runtime_error(error);
                    };
    
                    _dbg_ << "start import " << serv.name << " from " << serv.pier;
                    plexus::spawn_invite(m_job->io, opts, host, peer, on_invite, on_error);
                });
            }

        public:

            controller(boost::asio::io_context& io) : m_io(io), m_timer(io)
            {
            }

            ~controller()
            {
                stop();
            }

            void restart(const std::string& pier, const webpier::config& conf, const webpier::service& serv)
            {
                stop();

                conf.pier == pier
                    ? start_export(conf, serv) 
                    : start_import(conf, serv);
            }

            void suspend()
            {
                stop();

                for(auto& item : m_pool)
                    item.second.terminate();
            }

            health::status state() const
            {
                if (!m_job)
                    return health::asleep;

                if (m_job->future.valid() && m_job->future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready && !m_job->future.get().empty())
                    return health::broken;

                return m_pool.empty() ? health::lonely : health::burden;
            }

            std::string message() const
            {
                if (m_job && m_job->future.valid() && m_job->future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
                    return m_job->future.get();
                return "";
            }

            std::vector<report::tunnel> tunnels()
            {
                std::vector<report::tunnel> res;

                for(auto& item : m_pool)
                    res.emplace_back(report::tunnel{ item.first, static_cast<uint32_t>(item.second.id()) });

                return std::vector<report::tunnel>(std::move(res));
            }
        };

        class engine
        {
            boost::asio::io_context& m_io;
            std::filesystem::path m_home;
            std::map<handle, std::shared_ptr<controller>> m_pool;

            struct quard
            {
                quard(const std::filesystem::path& file)
                    : m_file(open_file_lock(file))
                    , m_lock(m_file)
                {
                }
    
            private:
    
                static boost::interprocess::file_lock open_file_lock(const std::filesystem::path& file)
                {
                    if (!std::filesystem::exists(file))
                        std::ofstream(file).close();
    
                    return boost::interprocess::file_lock(file.string().c_str());
                }
    
                boost::interprocess::file_lock m_file;
                boost::interprocess::scoped_lock<boost::interprocess::file_lock> m_lock;
            };

            webpier::config load_config() noexcept(false)
            {
                auto file = m_home / webpier_conf_file_name;

                boost::property_tree::ptree doc;
                boost::property_tree::read_json(file.string(), doc);

                auto folder = webpier::utf8_to_locale(doc.get<std::string>("log.folder", ""));
                auto level = webpier::journal::severity(doc.get<int>("log.level", webpier::journal::info));

                wormhole::log::set(wormhole::log::severity(level), make_log_path(folder));

                return webpier::config {
                    webpier::utf8_to_locale(doc.get<std::string>("pier")),
                    webpier::utf8_to_locale(doc.get<std::string>("repo")),
                    webpier::journal { folder, level },
                    webpier::puncher {
                        webpier::utf8_to_locale(doc.get<std::string>("nat.stun")),
                        doc.get<uint8_t>("nat.hops", 7)
                    },
                    webpier::dhtnode {
                        webpier::utf8_to_locale(doc.get<std::string>("dht.bootstrap", "")),
                        doc.get<uint16_t>("dht.port", 0),
                        0
                    },
                    webpier::emailer {
                        webpier::utf8_to_locale(doc.get<std::string>("email.smtp", "")),
                        webpier::utf8_to_locale(doc.get<std::string>("email.imap", "")),
                        webpier::utf8_to_locale(doc.get<std::string>("email.login", "")),
                        webpier::utf8_to_locale(doc.get<std::string>("email.password", "")),
                        webpier::utf8_to_locale(doc.get<std::string>("email.cert", "")),
                        webpier::utf8_to_locale(doc.get<std::string>("email.key", "")),
                        webpier::utf8_to_locale(doc.get<std::string>("email.ca", ""))
                    }
                };
            }

            webpier::service load_config(const std::filesystem::path& repo, const slipway::handle& id) noexcept(false)
            {
                auto path = repo / id.pier / webpier_conf_file_name;
                if (std::filesystem::exists(path))
                {
                    boost::property_tree::ptree doc;
                    boost::property_tree::read_json(path.string(), doc);

                    boost::property_tree::ptree array;
                    for (auto& item : doc.get_child("services", array))
                    {
                        if (webpier::utf8_to_locale(item.second.get<std::string>("name")) == id.service)
                        {
                            return webpier::service {
                                webpier::utf8_to_locale(item.second.get<std::string>("name")),
                                webpier::utf8_to_locale(item.second.get<std::string>("pier")),
                                webpier::utf8_to_locale(item.second.get<std::string>("address")),
                                webpier::utf8_to_locale(item.second.get<std::string>("gateway", webpier::default_gateway)),
                                webpier::utf8_to_locale(item.second.get<std::string>("rendezvous", "")),
                                item.second.get<bool>("autostart", false),
                                item.second.get<bool>("obscure", true),
                            };
                        }
                    }
                }

                return webpier::service{};
            }

            std::map<std::string, std::vector<webpier::service>> load_config(const std::filesystem::path& repo) noexcept(false)
            {
                std::map<std::string, std::vector<webpier::service>> res;
                for (auto const& owner : std::filesystem::directory_iterator(repo))
                {
                    if (!owner.is_directory())
                        continue;

                    for (auto const& pin : std::filesystem::directory_iterator(owner.path()))
                    {
                        if (!pin.is_directory())
                            continue;

                        auto conf = pin.path() / webpier_conf_file_name;
                        if (!std::filesystem::exists(conf))
                            continue;

                        auto pier = owner.path().filename().string() + "/" + pin.path().filename().string();

                        boost::property_tree::ptree doc;
                        boost::property_tree::read_json(conf.string(), doc);

                        boost::property_tree::ptree array;
                        for (auto& item : doc.get_child("services", array))
                        {
                            res[pier].emplace_back(webpier::service {
                                webpier::utf8_to_locale(item.second.get<std::string>("name")),
                                webpier::utf8_to_locale(item.second.get<std::string>("pier")),
                                webpier::utf8_to_locale(item.second.get<std::string>("address")),
                                webpier::utf8_to_locale(item.second.get<std::string>("gateway", webpier::default_gateway)),
                                webpier::utf8_to_locale(item.second.get<std::string>("rendezvous", "")),
                                item.second.get<bool>("autostart", false),
                                item.second.get<bool>("obscure", true)
                            });
                        }
                    }
                }

                return std::map<std::string, std::vector<webpier::service>>(std::move(res));
            }

            void engage() noexcept(false)
            {
                quard lock(m_home / webpier_lock_file_name);

                webpier::config conf = load_config();

                _inf_ << "engage...";

                std::map<handle, std::shared_ptr<controller>> pool;
                for (const auto& pier : load_config(conf.repo))
                {
                    for (const auto& serv : pier.second)
                    {
                        handle id { pier.first, serv.name };

                        auto iter = m_pool.find(id);
                        if (iter != m_pool.end())
                        {
                            iter = pool.emplace(id, iter->second).first;
                            if (serv.autostart)
                            {
                                _inf_ << "restart " << pier.first << ":" << serv.name;
                                iter->second->restart(pier.first, conf, serv);
                            }
                            else if (!serv.autostart && iter->second->state() != slipway::health::asleep)
                            {
                                _inf_ << "suspend " << pier.first << ":" << serv.name;
                                iter->second->suspend();
                            }
                        }
                        else
                        {
                            iter = pool.emplace(id, std::make_shared<controller>(m_io)).first;
                            if (serv.autostart)
                            {
                                _inf_ << "restart " << pier.first << ":" << serv.name;
                                iter->second->restart(pier.first, conf, serv);
                            }
                            else
                            {
                                _inf_ << "suspend " << pier.first << ":" << serv.name;
                                iter->second->suspend();
                            }
                        }
                    }
                }
  
                std::swap(m_pool, pool);
                for (auto& item : pool)
                {
                    auto iter = m_pool.find(item.first);
                    if (iter == m_pool.end())
                        _inf_ << "remove " << item.first.pier << ":" << item.first.service;
                }
            }

            void adjust() noexcept(false)
            {
                quard lock(m_home / webpier_lock_file_name);

                webpier::config conf = load_config();

                _inf_ << "adjust...";

                std::map<handle, std::shared_ptr<controller>> pool;
                for (const auto& pier : load_config(conf.repo))
                {
                    for (const auto& serv : pier.second)
                    {
                        handle id { pier.first, serv.name };

                        auto iter = m_pool.find(id);
                        if (iter != m_pool.end())
                        {
                            iter = pool.emplace(id, iter->second).first;
                            if (iter->second->state() != slipway::health::asleep)
                            {
                                _inf_ << "restart " << pier.first << ":" << serv.name;
                                iter->second->restart(pier.first, conf, serv);
                            }
                        }
                        else
                        {
                            iter = pool.emplace(id, std::make_shared<controller>(m_io)).first;
                            if (serv.autostart)
                            {
                                _inf_ << "restart " << pier.first << ":" << serv.name;
                                iter->second->restart(pier.first, conf, serv);
                            }
                            else
                            {
                                _inf_ << "suspend " << pier.first << ":" << serv.name;
                                iter->second->suspend();
                            }
                        }
                    }
                }

                std::swap(m_pool, pool);
                for (auto& item : pool)
                {
                    auto iter = m_pool.find(item.first);
                    if (iter == m_pool.end())
                        _inf_ << "remove " << item.first.pier << ":" << item.first.service;
                }
            }

            void engage(const slipway::handle& id) noexcept(false)
            {
                quard lock(m_home / webpier_lock_file_name);

                webpier::config conf = load_config();
                webpier::service serv = load_config(conf.repo, id);

                auto iter = m_pool.find(id);
                if (serv.name.empty())
                {
                    if (iter != m_pool.end())
                    {
                        _inf_ << "remove " << id.pier << ":" << id.service;
                        m_pool.erase(iter);
                    }
                    throw std::runtime_error("wrong service");
                }

                if (iter == m_pool.end())
                    iter = m_pool.emplace(id, std::make_shared<controller>(m_io)).first;

                _inf_ << "restart " << id.pier << ":" << id.service;

                iter->second->restart(id.pier, conf, serv);
            }

            void adjust(const slipway::handle& id) noexcept(false)
            {
                quard lock(m_home / webpier_lock_file_name);

                webpier::config conf = load_config();
                webpier::service serv = load_config(conf.repo, id);

                auto iter = m_pool.find(id);
                if (serv.name.empty())
                {
                    if (iter != m_pool.end())
                    {
                        _inf_ << "remove " << id.pier << ":" << id.service;
                        m_pool.erase(iter);
                    }
                    return;
                }

                if (iter == m_pool.end())
                {
                    iter = m_pool.emplace(id, std::make_shared<controller>(m_io)).first;
                    if (serv.autostart)
                    {
                        _inf_ << "restart " << id.pier << ":" << id.service;
                        iter->second->restart(id.pier, conf, serv);
                    }
                    else
                    {
                        _inf_ << "suspend " << id.pier << ":" << id.service;
                        iter->second->suspend();
                    }
                }
                else if (iter->second->state() != slipway::health::asleep)
                {
                    _inf_ << "restart " << id.pier << ":" << id.service;
                    iter->second->restart(id.pier, conf, serv);
                }
            }

            void unplug() noexcept(false)
            {
                quard lock(m_home / webpier_lock_file_name);

                webpier::config conf = load_config();

                _inf_ << "unplug...";

                std::map<handle, std::shared_ptr<controller>> pool;
                for (const auto& pier : load_config(conf.repo))
                {
                    for (const auto& serv : pier.second)
                    {
                        handle id { pier.first, serv.name };

                        auto iter = m_pool.find(id);
                        if (iter == m_pool.end())
                        {
                            iter = pool.emplace(id, std::make_shared<controller>(m_io)).first;
                            _inf_ << "suspend " << pier.first << ":" << serv.name;
                        }
                        else
                        {
                            iter = pool.emplace(id, iter->second).first;
                            if (iter->second->state() != slipway::health::asleep)
                            {
                                _inf_ << "suspend " << pier.first << ":" << serv.name;
                                iter->second->suspend();
                            }
                        }
                    }
                }

                std::swap(m_pool, pool);
                for (auto& item : pool)
                {
                    auto iter = m_pool.find(item.first);
                    if (iter == m_pool.end())
                        _inf_ << "remove " << item.first.pier << ":" << item.first.service;
                }
            }

            void unplug(const slipway::handle& id) noexcept(false)
            {
                quard lock(m_home / webpier_lock_file_name);

                webpier::config conf = load_config();
                webpier::service serv = load_config(conf.repo, id);

                auto iter = m_pool.find(id);
                if (serv.name.empty())
                {
                    if (iter != m_pool.end())
                    {
                        _inf_ << "remove " << id.pier << ":" << id.service;
                        m_pool.erase(iter);
                    }
                    return;
                }

                if (iter != m_pool.end())
                {
                    if (iter->second->state() != slipway::health::asleep)
                    {
                        _inf_ << "suspend " << id.pier << ":" << id.service;
                        iter->second->suspend();
                    }
                }
            }

            std::vector<slipway::health> status() noexcept(false)
            {
                std::vector<slipway::health> res;
                for (auto& item : m_pool)
                    res.emplace_back(report::health{ item.first, item.second->state(), item.second->message() });
                return res;
            }

            slipway::health status(const slipway::handle& id) noexcept(false)
            {
                auto iter = m_pool.find(id);
                if (iter != m_pool.end())
                    return slipway::health{ id, iter->second->state(), iter->second->message() };

                throw std::runtime_error("unknown service");
            }

            std::vector<slipway::report> report() noexcept(false)
            {
                std::vector<slipway::report> res;
                for (auto& item : m_pool)
                    res.emplace_back(slipway::report{ slipway::health{ item.first, item.second->state() }, item.second->tunnels() });
                return res;
            }

            slipway::report report(const slipway::handle& id) noexcept(false)
            {
                auto iter = m_pool.find(id);
                if (iter != m_pool.end())
                    return slipway::report { slipway::health { iter->first, iter->second->state() }, iter->second->tunnels() };

                throw std::runtime_error("unknown service");
            }

        public:

            engine(boost::asio::io_context& io, const std::filesystem::path& home)
                : m_io(io)
                , m_home(home)
            {
                engage();
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
                        case slipway::message::engage:
                        {
                            req.payload.index() == 1 
                                ? engage(std::get<slipway::handle>(req.payload)) 
                                : engage();
                            res = slipway::message::make(slipway::message::engage);
                            break;
                        }
                        case slipway::message::adjust:
                        {
                            req.payload.index() == 1 
                                ? adjust(std::get<slipway::handle>(req.payload)) 
                                : adjust();
                            res = slipway::message::make(slipway::message::adjust);
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
                    _err_ << ex.what();
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

            void cleanup(const std::string& socket)
            {
#ifdef WIN32
                DeleteFileA(socket.c_str());
#else
                ::unlink(socket.c_str());
#endif
            }

        public:

            server(boost::asio::io_context& io, const std::filesystem::path& socket, bool steady)
                : m_io(io)
                , m_acceptor(io, boost::asio::local::stream_protocol())
                , m_engine(io, socket.parent_path())
                , m_score(steady ? 1 : 0)
            {
                cleanup(socket.string());

                m_acceptor.bind(boost::asio::local::stream_protocol::endpoint(socket.u8string()));
                m_acceptor.listen();

                accept();
            }

            ~server()
            {
                cleanup(webpier::utf8_to_locale(m_acceptor.local_endpoint().path()));
            }
        };
    }
}

int main(int argc, char* argv[])
{
    try
    {
        if(webpier::get_module_path(SLIPWAY_MODULE) != argv[0])
        {
            std::cerr << "wrong module path" << std::endl;
            return 1;
        }

        if (argc < 2)
        {
            std::cerr << "no home argument" << std::endl;
            return 2;
        }

        std::filesystem::path home = std::filesystem::path(argv[1]);
        if (!std::filesystem::exists(home) || !std::filesystem::exists(home / slipway::webpier_conf_file_name))
        {
            std::cerr << "wrong home argument" << std::endl;
            return 3;
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
            return 4;
        }

        boost::asio::io_context io;
        slipway::server server(io, socket, argc == 3 && std::strcmp(argv[2], "daemon") == 0);
        io.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 5;
    }

    return 0;
}
