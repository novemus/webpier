#include "context.h"
#include "utils.h"
#include <map>
#include <fstream>
#include <filesystem>
#include <boost/property_tree/json_parser.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace webpier
{
    constexpr const char* cert_file_name = "cert.crt";
    constexpr const char* key_file_name = "private.key";
    constexpr const char* config_file_name = "webpier.json";
    constexpr const char* context_link_name = "context";

    using scoped_lock = boost::interprocess::scoped_lock<boost::interprocess::file_lock>;

    class guard
    {
        std::filesystem::path m_file;
        boost::interprocess::file_lock m_lock;
        std::filesystem::file_time_type m_time;

        static boost::interprocess::file_lock open_file_lock(const std::filesystem::path& file)
        {
            if (!std::filesystem::exists(file))
                std::ofstream(file).close();

            return boost::interprocess::file_lock(file.string().c_str());
        }

    public:

        guard(const std::filesystem::path& file)
            : m_file(file)
            , m_lock(open_file_lock(file))
        {
        }

        scoped_lock soft_lock()
        {
            scoped_lock lock(m_lock);

            if (m_time != std::filesystem::file_time_type() && m_time != std::filesystem::last_write_time(m_file))
                throw stale_error("context is outdated");

            return scoped_lock(std::move(lock));
        }

        scoped_lock hard_lock()
        {
            auto lock = soft_lock();

            m_time = std::filesystem::file_time_type::clock::now();
            std::filesystem::last_write_time(m_file, m_time);

            return scoped_lock(std::move(lock));
        }

        std::filesystem::path home() const
        {
            return m_file.parent_path();
        }
    };

    class context_impl : public context
    {
        using bundle = std::map<std::string, std::map<std::string, service>>;

        config m_config;
        bundle m_bundle;
        mutable guard m_guard;

        void load_config() noexcept(false)
        {
            try
            {
                auto file = m_guard.home() / config_file_name;
                if (std::filesystem::exists(file) && std::filesystem::file_size(file) > 0)
                {
                    boost::property_tree::ptree doc;
                    boost::property_tree::read_json(file.string(), doc);
                    m_config.pier = doc.get<std::string>("pier");
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
                auto file = m_guard.home() / config_file_name;

                boost::property_tree::ptree doc;
                doc.put("pier", m_config.pier);
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

                auto file = m_guard.home() / context_link_name / id / config_file_name;
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
                auto file = m_guard.home() / context_link_name / id / config_file_name;
                auto& services = m_bundle[id];

                boost::property_tree::ptree array;
                for (auto& unit : services)
                {
                    boost::property_tree::ptree item;
                    item.put("name", unit.second.name);
                    item.put("pier", unit.second.pier);
                    item.put("address", unit.second.address);
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
            : m_guard(home / config_file_name)
        {
            m_guard.soft_lock();

            auto conf = home / config_file_name;
            if (std::filesystem::exists(conf))
            {
                load_config();
            }

            auto repo = home / context_link_name;
            if (std::filesystem::exists(repo))
            {
                for (auto const& owner : std::filesystem::directory_iterator(repo))
                {
                    if (!owner.is_directory())
                        continue;

                    for (auto const& child : std::filesystem::directory_iterator(owner.path()))
                    {
                        if (!child.is_directory())
                            continue;

                        if (!std::filesystem::exists(child.path() / cert_file_name))
                            continue;

                        auto id = owner.path().filename().string() + "/" + child.path().filename().string();
                        load_pier_config(id);
                    }
                }
            }
        }

        void get_config(config& info) const noexcept(true) override
        {
            info = m_config; 
        }

        void set_config(const config& info) noexcept(false) override
        {
            if (info.pier != m_config.pier)
            {
                auto dir = to_hexadecimal(info.pier.data(), info.pier.size());
            
                auto cert = m_guard.home() / dir / info.pier / cert_file_name;
                auto key = m_guard.home() / dir / info.pier / key_file_name;

                bool wrong = (!std::filesystem::exists(cert) && std::filesystem::exists(key))
                          || (std::filesystem::exists(cert) && !std::filesystem::exists(key));

                if (wrong)
                    throw usage_error("wrong local pier");

                m_config = info;
                m_bundle.clear();
                m_bundle.emplace(m_config.pier, bundle::mapped_type());

                m_guard.hard_lock();

                if (!std::filesystem::exists(cert) && !std::filesystem::exists(key))
                {
                    std::filesystem::create_directories(cert.parent_path());
                    generate_x509_pair(cert, key, info.pier);
                }

                std::filesystem::remove(m_guard.home() / context_link_name);
                std::filesystem::create_directory_symlink(m_guard.home() / dir, m_guard.home() / context_link_name);

                save_config();
            }
            else
            {
                m_config = info;

                m_guard.hard_lock();
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
                throw usage_error("local pier does not exist");

            if (iter->second.find(info.name) != iter->second.end())
                throw usage_error("service already exists");

            iter->second.emplace(info.name, info);

            m_guard.hard_lock();
            save_pier_config(m_config.pier);
        }

        void del_export_service(const std::string& name) noexcept(false) override
        {
            auto iter = m_bundle.find(m_config.pier);
            if (iter == m_bundle.end())
                throw usage_error("local pier does not exist");

            if (iter->second.erase(name))
            {
                m_guard.hard_lock();
                save_pier_config(m_config.pier);
            }
        }

        void add_import_service(const service& info) noexcept(false) override
        {
            auto iter = m_bundle.find(info.pier);
            if (iter == m_bundle.end())
                throw usage_error("remote pier does not exist");

            if (iter->second.find(info.name) != iter->second.end())
                throw usage_error("service already exists");

            iter->second.emplace(info.name, info);

            m_guard.hard_lock();
            save_pier_config(info.pier);
        }

        void del_import_service(const std::string& pier, const std::string& name) noexcept(false) override
        {
            auto iter = m_bundle.find(pier);
            if (iter == m_bundle.end())
                throw usage_error("remote pier does not exist");

            if (iter->second.erase(name))
            {
                m_guard.hard_lock();
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

        void add_pier(const std::string& id, const std::string& cert) noexcept(false) override
        {
            m_guard.hard_lock();

            auto path = m_guard.home() / context_link_name / id;

            if (std::filesystem::exists(path))
                throw usage_error("pier already exists");

            std::filesystem::create_directories(path);
            save_x509_cert(path / cert_file_name, cert);

            m_bundle.emplace(id, bundle::mapped_type());
        }

        void del_pier(const std::string& id) noexcept(false) override
        {
            if (m_config.pier == id)
                throw usage_error("can't delete local pier");

            m_guard.hard_lock();
            m_bundle.erase(id);

            try
            {
                std::filesystem::remove_all(m_guard.home() / context_link_name / id);
            }
            catch(const std::exception& e)
            {
                throw file_error(e.what());
            }
        }

        std::string get_fingerprint(const std::string& id) const noexcept(false) override
        {
            m_guard.soft_lock();
            return get_x509_public_sha1(m_guard.home() / context_link_name / id / cert_file_name);
        }

        std::string get_certificate(const std::string& id) const noexcept(false) override
        {
            m_guard.soft_lock();
            return load_x509_cert(m_guard.home() / context_link_name / id / cert_file_name);
        }
    };

    std::shared_ptr<context> open_context(const std::string& home) noexcept(false)
    {
        return std::make_shared<context_impl>(home);
    }
}
