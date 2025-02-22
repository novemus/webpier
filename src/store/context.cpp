#include <store/context.h>
#include <store/utils.h>
#include <map>
#include <fstream>
#include <filesystem>
#include <boost/property_tree/json_parser.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace webpier
{
    namespace
    {
        constexpr const char* cert_file_name = "cert.crt";
        constexpr const char* key_file_name = "private.key";
        constexpr const char* conf_file_name = "webpier.json";
        constexpr const char* lock_file_name = "webpier.lock";

        struct locker
        {
            locker(const std::filesystem::path& file)
                : m_file(file)
                , m_lock(open_file_lock(file))
            {
            }

            std::filesystem::path home() const
            {
                return m_file.parent_path();
            }

        private:

            static boost::interprocess::file_lock open_file_lock(const std::filesystem::path& file)
            {
                if (!std::filesystem::exists(file))
                    std::ofstream(file).close();

                return boost::interprocess::file_lock(file.string().c_str());
            }

            friend struct soft_lock;
            friend struct hard_lock;

            std::filesystem::path           m_file;
            boost::interprocess::file_lock  m_lock;
            std::filesystem::file_time_type m_time;
        };

        struct soft_lock
        {
            soft_lock(locker& guard) 
                : m_lock(guard.m_lock)
            {
                if (guard.m_time != std::filesystem::file_time_type() && guard.m_time != std::filesystem::last_write_time(guard.m_file))
                    throw stale_error("The context is outdated");
            }

            virtual ~soft_lock() { }

        private:

            boost::interprocess::scoped_lock<boost::interprocess::file_lock> m_lock;
        };

        struct hard_lock : public soft_lock
        {
            hard_lock(locker& guard) 
                : soft_lock(guard)
                , m_guard(guard)
            {
            }

            ~hard_lock()
            {
                m_guard.m_time = std::filesystem::file_time_type::clock::now();
                std::filesystem::last_write_time(m_guard.m_file, m_guard.m_time);
            }

        private:

            locker& m_guard;
        };

        class context_impl : public context
        {
            using bundle = std::map<std::string, std::map<std::string, service>>;

            config m_config;
            bundle m_bundle;
            mutable locker m_guard;

            void load_config() noexcept(false)
            {
                try
                {
                    auto file = m_guard.home() / conf_file_name;
                    if (std::filesystem::exists(file) && std::filesystem::file_size(file) > 0)
                    {
                        boost::property_tree::ptree doc;
                        boost::property_tree::read_json(file.string(), doc);
                        m_config.pier = doc.get<std::string>("pier");
                        m_config.repo = doc.get<std::string>("repo");
                        m_config.log.folder = doc.get<std::string>("log.folder", "");
                        m_config.log.level = journal::severity(doc.get<int>("log.level", journal::info));
                        m_config.nat.hops = doc.get<uint8_t>("nat.hops", 7);
                        m_config.nat.stun = doc.get<std::string>("nat.stun", default_stun_server);
                        m_config.nat.hops = doc.get<uint8_t>("nat.hops", 7);
                        m_config.dht.bootstrap = doc.get<std::string>("dht.bootstrap", default_dht_bootstrap);
                        m_config.dht.port = doc.get<uint16_t>("dht.port", default_dht_port);
                        m_config.email.smtp = doc.get<std::string>("email.smtp", "");
                        m_config.email.imap = doc.get<std::string>("email.imap", "");
                        m_config.email.login = doc.get<std::string>("email.login", "");
                        m_config.email.password = doc.get<std::string>("email.password", "");
                        m_config.email.cert = doc.get<std::string>("email.cert", "");
                        m_config.email.key = doc.get<std::string>("email.key", "");
                        m_config.email.ca = doc.get<std::string>("email.ca", "");
                        m_config.autostart = doc.get<bool>("autostart", false);
                    }
                }
                catch(const std::exception& e)
                {
                    throw file_error(e.what());
                }
            }

            void save_config() noexcept(false)
            {
                try
                {
                    auto file = m_guard.home() / conf_file_name;

                    boost::property_tree::ptree doc;
                    doc.put("pier", m_config.pier);
                    doc.put("repo", m_config.repo);
                    doc.put("log.folder", m_config.log.folder);
                    doc.put("log.level", m_config.log.level);
                    doc.put("nat.stun", m_config.nat.stun);
                    doc.put("nat.hops", m_config.nat.hops);
                    doc.put("dht.bootstrap", m_config.dht.bootstrap);
                    doc.put("dht.port", m_config.dht.port);
                    doc.put("emailer.smtp", m_config.email.smtp);
                    doc.put("emailer.imap", m_config.email.imap);
                    doc.put("emailer.login", m_config.email.login);
                    doc.put("emailer.password", m_config.email.password);
                    doc.put("emailer.cert", m_config.email.cert);
                    doc.put("emailer.key", m_config.email.key);
                    doc.put("emailer.ca", m_config.email.ca);
                    doc.put("autostart", m_config.autostart);
                    boost::property_tree::write_json(file.string(), doc);
                }
                catch(const std::exception& e)
                {
                    throw file_error(e.what());
                }
            }

            void load_pier_config(const std::string& id) noexcept(false)
            {
                try
                {
                    auto& services = m_bundle[id];

                    auto file = std::filesystem::path(m_config.repo) / id / conf_file_name;
                    if (std::filesystem::exists(file))
                    {
                        boost::property_tree::ptree doc;
                        boost::property_tree::read_json(file.string(), doc);

                        boost::property_tree::ptree array;
                        for (auto& item : doc.get_child("services", array))
                        {
                            service unit;
                            unit.name = item.second.get<std::string>("name", "");
                            unit.pier = item.second.get<std::string>("pier", "");
                            unit.address = item.second.get<std::string>("address", "");
                            unit.gateway = item.second.get<std::string>("gateway", default_gateway);
                            unit.rendezvous = item.second.get<std::string>("rendezvous", "");
                            unit.autostart = item.second.get<bool>("autostart", false);
                            unit.obscure = item.second.get<bool>("obscure", true);
                            services.emplace(unit.name, unit);
                        }
                    }
                }
                catch(const std::exception& e)
                {
                    throw file_error(e.what());
                }
            }

            void save_pier_config(const std::string& id) noexcept(false)
            {
                try
                {
                    auto file = std::filesystem::path(m_config.repo) / id / conf_file_name;
                    auto& services = m_bundle[id];

                    boost::property_tree::ptree array;
                    for (auto& unit : services)
                    {
                        boost::property_tree::ptree item;
                        item.put("name", unit.second.name);
                        item.put("pier", unit.second.pier);
                        item.put("address", unit.second.address);
                        item.put("gateway", unit.second.gateway);
                        item.put("autostart", unit.second.autostart);
                        item.put("obscure", unit.second.obscure);
                        item.put("rendezvous", unit.second.rendezvous);
                        array.push_back(std::make_pair("", item));
                    }

                    boost::property_tree::ptree doc;
                    doc.put_child("services", array);

                    boost::property_tree::write_json(file.string(), doc);
                }
                catch(const std::exception& e)
                {
                    throw file_error(e.what());
                }
            }

        public:

            context_impl(const std::filesystem::path& home)
                : m_guard(home / lock_file_name)
            {
                soft_lock lock(m_guard);

                if (std::filesystem::exists(home / conf_file_name))
                {
                    load_config();

                    if (std::filesystem::exists(m_config.repo))
                    {
                        for (auto const& owner : std::filesystem::directory_iterator(m_config.repo))
                        {
                            if (!owner.is_directory())
                                continue;

                            for (auto const& pin : std::filesystem::directory_iterator(owner.path()))
                            {
                                if (!pin.is_directory())
                                    continue;

                                if (!std::filesystem::exists(pin.path() / cert_file_name))
                                    continue;

                                auto id = owner.path().filename().string() + "/" + pin.path().filename().string();
                                load_pier_config(id);
                            }
                        }
                    }
                }
            }

            std::string pier() const noexcept(true) override
            {
                return m_config.pier;
            }

            std::string home() const noexcept(true) override
            {
                return m_guard.home().generic_u8string();
            }

            void get_config(config& info) const noexcept(true) override
            {
                info = m_config; 
            }

            void set_config(const config& info) noexcept(false) override
            {
                if (info.pier != m_config.pier)
                {
                    auto home = std::filesystem::path(info.repo) / info.pier;
                    auto cert = home / cert_file_name;
                    auto key = home / key_file_name;

                    bool wrong = (!std::filesystem::exists(cert) && std::filesystem::exists(key))
                            || (std::filesystem::exists(cert) && !std::filesystem::exists(key));

                    if (wrong)
                        throw usage_error("Wrong local pier");

                    m_config = info;
                    m_bundle.clear();
                    m_bundle.emplace(m_config.pier, bundle::mapped_type());

                    hard_lock lock(m_guard);

                    std::filesystem::create_directories(m_config.repo);
                    std::filesystem::create_directories(m_config.log.folder);

                    if (!std::filesystem::exists(cert) && !std::filesystem::exists(key))
                    {
                        std::filesystem::create_directories(cert.parent_path());
                        generate_x509_pair(cert, key, info.pier);
                    }

                    save_config();
                }
                else
                {
                    m_config = info;

                    hard_lock lock(m_guard);
                    save_config();
                }
            }

            void get_export_services(std::vector<service>& list) const noexcept(true) override
            {
                auto iter = m_bundle.find(m_config.pier);
                if (iter != m_bundle.end())
                {
                    for(auto& item : iter->second)
                        list.push_back(item.second);
                }
            }

            void get_import_services(std::vector<service>& list) const noexcept(true) override
            {
                for (const auto& pier : m_bundle)
                {
                    if (pier.first != m_config.pier)
                    {
                        for(auto& item : pier.second)
                            list.push_back(item.second);
                    }
                }
            }

            void add_export_service(const service& info) noexcept(false) override
            {
                auto iter = m_bundle.find(m_config.pier);
                if (iter == m_bundle.end())
                    throw usage_error("The local pier does not exist");

                if (iter->second.find(info.name) != iter->second.end())
                    throw usage_error("Such service already exists");

                iter->second.emplace(info.name, info);

                hard_lock lock(m_guard);
                save_pier_config(m_config.pier);
            }

            void del_export_service(const std::string& name) noexcept(false) override
            {
                auto iter = m_bundle.find(m_config.pier);
                if (iter == m_bundle.end())
                    throw usage_error("The local pier does not exist");

                if (iter->second.erase(name))
                {
                    hard_lock lock(m_guard);
                    save_pier_config(m_config.pier);
                }
            }

            void add_import_service(const service& info) noexcept(false) override
            {
                auto iter = m_bundle.find(info.pier);
                if (iter == m_bundle.end())
                    throw usage_error("The remote pier does not exist");

                if (iter->second.find(info.name) != iter->second.end())
                    throw usage_error("Such service already exists");

                iter->second.emplace(info.name, info);

                hard_lock lock(m_guard);
                save_pier_config(info.pier);
            }

            void del_import_service(const std::string& pier, const std::string& name) noexcept(false) override
            {
                auto iter = m_bundle.find(pier);
                if (iter == m_bundle.end())
                    throw usage_error("The remote pier does not exist");

                if (iter->second.erase(name))
                {
                    hard_lock lock(m_guard);
                    save_pier_config(pier);
                }
            }

            void get_piers(std::vector<std::string>& list) const noexcept(true) override
            {
                for (const auto& item : m_bundle)
                {
                    if (item.first != m_config.pier)
                        list.push_back(item.first);
                }
            }

            void add_pier(const std::string& pier, const std::string& cert) noexcept(false) override
            {
                hard_lock lock(m_guard);

                auto path = std::filesystem::path(m_config.repo) / pier;

                if (std::filesystem::exists(path))
                    throw usage_error("The pier already exists");

                std::filesystem::create_directories(path);
                save_x509_cert(path / cert_file_name, cert);

                m_bundle.emplace(pier, bundle::mapped_type());
            }

            void del_pier(const std::string& pier) noexcept(false) override
            {
                if (m_config.pier == pier)
                    throw usage_error("Can't delete local pier");

                hard_lock lock(m_guard);
                m_bundle.erase(pier);

                try
                {
                    std::filesystem::remove_all(std::filesystem::path(m_config.repo) / pier);
                }
                catch(const std::exception& e)
                {
                    throw file_error(e.what());
                }
            }

            std::string get_fingerprint(const std::string& pier) const noexcept(false) override
            {
                soft_lock lock(m_guard);
                return get_x509_public_sha1(std::filesystem::path(m_config.repo) / pier / cert_file_name);
            }

            std::string get_certificate(const std::string& pier) const noexcept(false) override
            {
                soft_lock lock(m_guard);
                return load_x509_cert(std::filesystem::path(m_config.repo) / pier / cert_file_name);
            }
        };
    }

    std::shared_ptr<context> open_context(const std::string& home) noexcept(false)
    {
        return std::make_shared<context_impl>(home);
    }
}
