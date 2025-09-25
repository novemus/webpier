#include <backend/server.h>
#include <backend/message.h>
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
#include <boost/process/v2/execute.hpp>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
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
        constexpr const char* webpier_conf_file_name = "webpier.json";
        constexpr const char* webpier_lock_file_name = "webpier.lock";
        constexpr const char* slipway_jack_file_name = "slipway.jack";

        namespace utils
        {
            boost::posix_time::seconds get_retry_timeout() noexcept(true)
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

            plexus::options make_options(const webpier::config& config, const webpier::service& service) noexcept(false)
            {
                return plexus::options {
                    service.name,
                    config.repo,
                    webpier::make_udp_endpoint(config.nat.stun, webpier::stun_server_default_port),
                    webpier::make_udp_endpoint(service.gateway, webpier::stun_client_default_port),
                    config.nat.hops,
                    service.rendezvous.empty()
                        ? plexus::rendezvous {
                            plexus::emailer {
                                webpier::make_tcp_endpoint(config.email.smtp, webpier::smtp_server_default_port),
                                webpier::make_tcp_endpoint(config.email.imap, webpier::imap_server_default_port),
                                config.email.login,
                                config.email.password,
                                config.email.cert,
                                config.email.key,
                                config.email.ca 
                            }}
                        : plexus::rendezvous {
                            plexus::dhtnode {
                                config.dht.bootstrap + "," + service.rendezvous,
                                config.dht.port,
                                config.dht.network
                            }}
                    };
            }

            plexus::identity make_identity(const std::string& pier) noexcept(true)
            {
                static constexpr const char SLASH = '/';
                size_t pos = pier.find(SLASH);

                return pos != std::string::npos
                    ? plexus::identity { pier.substr(0, pos), pier.substr(pos + 1) } 
                    : plexus::identity { pier, "" };
            }

            std::string stringify(const plexus::identity& id) noexcept(true)
            {
                return id.owner + "/" + id.pin;
            }

            std::string stringify(const boost::asio::ip::udp::endpoint& ep) noexcept(true)
            {
                return ep.address().to_string() + ":" + std::to_string(ep.port());
            }

            std::string make_log_path(const std::string& folder) noexcept(true)
            {
                if (folder.empty())
                    return "";

                return folder + webpier::make_timestamp("/slipway.%Y%m%d.log");
            }
        }

        class controller : public std::enable_shared_from_this<controller>
        {
            class connector : public std::enable_shared_from_this<connector>
            {
                using weak_ptr = std::weak_ptr<connector>;
                using signal_ptr = std::shared_ptr<boost::asio::cancellation_signal>;

                class spawner
                {
                    struct
                    {
                        webpier::config   config;
                        webpier::service  service;
                        plexus::connector connect;
                        plexus::fallback  fallback;
                    } 
                    m_data;

                    std::unique_ptr<boost::asio::io_context> m_io;
                    std::unique_ptr<std::thread>             m_thread;

                public:

                    spawner(const webpier::config& config, const webpier::service& service, const plexus::connector& connect, const plexus::fallback& fallback)
                    {
                        m_data.config = config;
                        m_data.service = service;
                        m_data.connect = connect;
                        m_data.fallback = fallback;
                    }

                    ~spawner()
                    {
                        complete();
                    }

                    void complete()
                    {
                        if (m_io)
                            m_io->stop();

                        if (m_thread && m_thread->joinable())
                            m_thread->join();
                    }

                    void startup()
                    {
                        complete();

                        m_io = std::make_unique<boost::asio::io_context>();

                        m_thread = std::make_unique<std::thread>([this]()
                        {
                            auto host = utils::make_identity(m_data.config.pier);
                            auto peer = utils::make_identity(m_data.service.pier);

                            try
                            {
                                auto config = utils::make_options(m_data.config, m_data.service);

                                m_data.service.local
                                    ? plexus::spawn_accept(*m_io, config, host, peer, m_data.connect, m_data.fallback)
                                    : plexus::spawn_invite(*m_io, config, host, peer, m_data.connect, m_data.fallback);

                                m_io->run();
                            }
                            catch(const std::exception& ex)
                            {
                                m_data.fallback(host, peer, ex.what());
                            }
                        });
                    }
                };

                void connect(const boost::asio::ip::udp::endpoint& bind, const plexus::reference& host, const plexus::reference& peer)
                {
                    boost::process::v2::process proc(m_io, webpier::get_module_path(webpier::carrier_module).string(), 
                    {
                        "--purpose=" + std::string(m_service.local ? "export" : "import"),
                        "--service=" + m_service.address,
                        "--gateway=" + utils::stringify(bind),
                        "--faraway=" + utils::stringify(peer.endpoint),
                        "--obscure=" + std::to_string(m_service.obscure ? host.puzzle ^ peer.puzzle : 0),
                        "--journal=" + (m_config.log.folder.empty() ? "" : m_config.log.folder + "/carrier.%p.log"),
                        "--logging=" + std::to_string(m_config.log.level)
                    });

                    int pid = proc.id();
 
                    m_service.local
                        ? _inf_ << "starting " << pid << " export tunnel " << m_config.pier << ":" << m_service.name << " -> " << m_service.pier
                        : _inf_ << "starting " << pid << " import tunnel " << m_service.pier << ":" << m_service.name << " -> " << m_config.pier;

                    auto signal = std::make_shared<boost::asio::cancellation_signal>();
                    m_tunnels.emplace(pid, signal);

                    weak_ptr weak = shared_from_this();
                    boost::process::v2::async_execute(std::move(proc), boost::asio::bind_cancellation_slot(signal->slot(),
                        [this, weak, signal, pid](const boost::system::error_code& ec, int code)
                        {
                            if (ec && ec != boost::asio::error::operation_aborted)
                                _err_ << ec.message();

                            _inf_ << "joined " << pid << " tunnel with exit code " << code;

                            if (auto ptr = weak.lock())
                            {
                                m_tunnels.erase(pid);

                                if (m_service.local == false)
                                {
                                    m_error.clear();
                                    m_spawner->startup();
                                }
                            }
                        }));
                }

                void fallback(const std::string& error)
                {
                    m_error = error;

                    m_service.local
                        ? _err_ << "export service " << m_config.pier << ":" << m_service.name << " -> " << m_service.pier << " failed: " << error
                        : _err_ << "import service " << m_service.pier << ":" << m_service.name << " -> " << m_config.pier << " failed: " << error;

                    weak_ptr weak = shared_from_this();

                    m_timer.expires_from_now(utils::get_retry_timeout());
                    m_timer.async_wait([this, weak](const boost::system::error_code& ec)
                    {
                        if (ec)
                            return;

                        if(auto ptr = weak.lock())
                        {
                            m_error.clear();
                            m_spawner->startup();
                        }
                    });
                }

            public:

                connector(boost::asio::io_context& io)
                    : m_io(io)
                    , m_timer(io)
                {
                }

                ~connector()
                {
                    boost::system::error_code ec;
                    m_timer.cancel(ec);

                    if (m_spawner)
                        m_spawner->complete();

                    for(auto& item : m_tunnels)
                    {
                        _inf_ << "terminate " << item.first << " tunnel";
                        item.second->emit(boost::asio::cancellation_type::terminal);
                    }
                }

                void restart(const webpier::config& config, const webpier::service& service)
                {
                    boost::system::error_code ec;
                    m_timer.cancel(ec);

                    if (m_spawner)
                        m_spawner->complete();

                    m_config = config;
                    m_service = service;

                    weak_ptr weak = shared_from_this();

                    auto connect = [this, weak](const plexus::identity&, const plexus::identity&, const boost::asio::ip::udp::endpoint& bind, const plexus::reference& host, const plexus::reference& peer)
                    {
                        if(auto ptr = weak.lock())
                        {
                            boost::asio::post(m_io, [weak, bind, host, peer]()
                            {
                                if(auto ptr = weak.lock())
                                    ptr->connect(bind, host, peer);
                            });
                        }
                    };

                    auto fallback = [this, weak](const plexus::identity&, const plexus::identity&, const std::string& error)
                    {
                        if(auto ptr = weak.lock())
                        {
                            boost::asio::post(m_io, [weak, error]()
                            {
                                if(auto ptr = weak.lock())
                                    ptr->fallback(error);
                            });
                        }
                    };

                    m_spawner = std::make_unique<spawner>(m_config, m_service, connect, fallback);

                    if (m_service.local || m_tunnels.empty())
                    {
                        m_error.clear();
                        m_spawner->startup();
                    }
                }

                bool broken() const
                {
                    return !m_error.empty();
                }

                bool burden() const
                {
                    return !m_tunnels.empty();
                }

                std::string error() const
                {
                    return m_error;
                }

                std::vector<uint32_t> tunnels() const
                {
                    std::vector<uint32_t> res;

                    for(auto& item : m_tunnels)
                        res.emplace_back(static_cast<uint32_t>(item.first));
    
                    return std::vector<uint32_t>(std::move(res));
                }

            private:

                boost::asio::io_context&    m_io;
                boost::asio::deadline_timer m_timer;
                webpier::config             m_config;
                webpier::service            m_service;
                std::unique_ptr<spawner>    m_spawner;
                std::string                 m_error;
                std::map<int, signal_ptr>   m_tunnels;
            };

        public:

            controller(boost::asio::io_context& io)
                : m_io(io)
            {
            }

            void restart(const webpier::config& config, const webpier::service& service)
            {
                std::set<std::string> piers;
                boost::split(piers, service.pier, boost::is_any_of(" "));

                auto iter = m_bundle.begin();
                while (iter != m_bundle.end())
                {
                    if (piers.find(iter->first) == piers.end())
                        iter = m_bundle.erase(iter);
                    else
                        ++iter;
                }

                for (auto& pier : piers)
                {
                    auto single = service;
                    single.pier = pier;

                    auto iter = m_bundle.find(pier);
                    if (iter == m_bundle.end())
                        iter = m_bundle.emplace(pier, std::make_shared<connector>(m_io)).first;

                    iter->second->restart(config, single);
                }
            }

            void suspend()
            {
                m_bundle.clear();
            }

            health::status state() const
            {
                health::status res = m_bundle.empty() ? health::asleep : health::lonely;
                for(auto& item : m_bundle)
                {
                    if (item.second->broken())
                    {
                        res = health::broken;
                        break;
                    }
                    else if (item.second->burden())
                    {
                        res = health::burden;
                        break;
                    }
                }
                return res;
            }

            std::string message() const
            {
                for(auto& item : m_bundle)
                {
                    if (item.second->broken())
                        return item.second->error();
                }
                return "";
            }

            std::vector<report::tunnel> tunnels()
            {
                std::vector<report::tunnel> res;
                for(auto& item : m_bundle)
                {
                    for(auto& pid : item.second->tunnels())
                        res.emplace_back(report::tunnel{ item.first, pid });
                }
                return std::vector<report::tunnel>(std::move(res));
            }

        private:

            boost::asio::io_context& m_io;
            std::map<std::string, std::shared_ptr<connector>> m_bundle;
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

                wormhole::log::set(wormhole::log::severity(level), utils::make_log_path(folder));

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
                                item.second.get<bool>("local"),
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
                                item.second.get<bool>("local"),
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
                                _inf_ << "restart " << id.pier << ":" << id.service;
                                iter->second->restart(conf, serv);
                            }
                            else if (!serv.autostart && iter->second->state() != slipway::health::asleep)
                            {
                                _inf_ << "suspend " << id.pier << ":" << id.service;
                                iter->second->suspend();
                            }
                        }
                        else
                        {
                            iter = pool.emplace(id, std::make_shared<controller>(m_io)).first;
                            if (serv.autostart)
                            {
                                _inf_ << "restart " << id.pier << ":" << id.service;
                                iter->second->restart(conf, serv);
                            }
                            else
                            {
                                _inf_ << "suspend " << id.pier << ":" << id.service;
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
                                _inf_ << "restart " << id.pier << ":" << id.service;
                                iter->second->restart(conf, serv);
                            }
                        }
                        else
                        {
                            iter = pool.emplace(id, std::make_shared<controller>(m_io)).first;
                            if (serv.autostart)
                            {
                                _inf_ << "restart " << id.pier << ":" << id.service;
                                iter->second->restart(conf, serv);
                            }
                            else
                            {
                                _inf_ << "suspend " << id.pier << ":" << id.service;
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

                iter->second->restart(conf, serv);
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
                        iter->second->restart(conf, serv);
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
                    iter->second->restart(conf, serv);
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
            }

            void launch() noexcept(false)
            {
                engage();
            }

            void comply(boost::asio::streambuf& request) noexcept(true)
            {
                slipway::message req, res;

                try
                {
                    slipway::pull_message(request, req);

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

                slipway::push_message(request, res);
            }
        };

        class server_impl : public server
        {
            boost::asio::io_context m_io;
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

                    m_engine.comply(buffer);

                    boost::asio::async_write(socket, buffer, yield[ec]);
                    if (ec)
                        return cleanup();

                    handle(std::move(socket));
                }, boost::asio::detached);
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

            server_impl(const std::filesystem::path& home, bool steady)
                : m_acceptor(m_io, boost::asio::local::stream_protocol())
                , m_engine(m_io, home)
                , m_score(steady ? 1 : 0)
            {
                auto dir = std::filesystem::temp_directory_path() / std::to_string(std::hash<std::string>()(home.string()));
                std::filesystem::create_directory(dir);

                auto socket = dir / slipway_jack_file_name;

                cleanup(socket.string());

                m_acceptor.bind(boost::asio::local::stream_protocol::endpoint(socket.u8string()));
                m_acceptor.listen();
            }

            ~server_impl()
            {
                cleanup(webpier::utf8_to_locale(m_acceptor.local_endpoint().path()));
            }

            void employ() noexcept(false) override
            {
                m_io.restart();
                m_engine.launch();
    
                accept();
    
                m_io.run();
            }

            void cancel() noexcept(true) override
            {
                boost::system::error_code ec;
                m_acceptor.cancel(ec);

                m_io.stop();
            }
        };
    }

    std::shared_ptr<server> create_backend(const std::string& home, bool steady) noexcept(false)
    {
        return std::make_shared<server_impl>(home, steady);
    }
}
