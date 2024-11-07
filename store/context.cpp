#include "context.h"
#include "utils.h"
#include <map>
#include <filesystem>
#include <openssl/x509v3.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/file_lock.hpp>

namespace webpier
{
    constexpr const char* cert_file_name = "cert.crt";
    constexpr const char* key_file_name = "private.key";
    constexpr const char* conf_file_name = "webpier.json";
    constexpr const char* lock_file_name = "lock";
    constexpr const char* repo_dir_name = "repo";

    class webpier
    {
        config m_config;
        std::filesystem::path m_path;
        boost::interprocess::file_lock m_locker;
        std::filesystem::file_time_type m_timestamp;

        void load() noexcept(false)
        {
            try
            {
                boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker, boost::interprocess::try_to_lock_type());

                boost::property_tree::ptree doc;
                boost::property_tree::read_json(m_path.string(), doc);

                m_config.host = doc.get<std::string>("host");
                m_config.traverse.stun = doc.get<std::string>("nat.traverse.stun", default_stun_server);
                m_config.traverse.hops = doc.get<uint8_t>("nat.traverse.hops", 7);
                m_config.rendezvous.bootstrap = doc.get<std::string>("rendezvous.dht.bootstrap", default_dht_bootstrap);
                m_config.rendezvous.network = doc.get<uint32_t>("rendezvous.dht.network", 0);
                m_config.emailer.smtp = doc.get<std::string>("emailer.smtp", "");
                m_config.emailer.imap = doc.get<std::string>("emailer.imap", "");
                m_config.emailer.login = doc.get<std::string>("emailer.login", "");
                m_config.emailer.password = doc.get<std::string>("emailer.password", "");
                m_config.emailer.cert = doc.get<std::string>("emailer.cert", "");
                m_config.emailer.key = doc.get<std::string>("emailer.key", "");
                m_config.emailer.ca = doc.get<std::string>("emailer.ca", "");

                m_timestamp = std::filesystem::last_write_time(m_path);
            }
            catch(const boost::interprocess::interprocess_exception& e)
            {
                throw lock_error(e.what());
            }
            catch(const std::exception& e)
            {
                throw file_error(e.what());
            }
        }

        void save() noexcept(false)
        {
            try
            {
                boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker, boost::interprocess::try_to_lock_type());

                if (m_timestamp == std::filesystem::file_time_type() || std::filesystem::last_write_time(m_path) == m_timestamp)
                {
                    boost::property_tree::ptree doc;

                    doc.put("host", m_config.host);
                    doc.put("nat.traverse.stun", m_config.traverse.stun);
                    doc.put("nat.traverse.hops", m_config.traverse.hops);
                    doc.put("rendezvous.dht.bootstrap", m_config.rendezvous.bootstrap);
                    doc.put("rendezvous.dht.network", m_config.rendezvous.network);
                    doc.put("emailer.smtp", m_config.emailer.smtp);
                    doc.put("emailer.imap", m_config.emailer.imap);
                    doc.put("emailer.login", m_config.emailer.login);
                    doc.put("emailer.password", m_config.emailer.password);
                    doc.put("emailer.cert", m_config.emailer.cert);
                    doc.put("emailer.key", m_config.emailer.key);
                    doc.put("emailer.ca", m_config.emailer.ca);

                    boost::property_tree::write_json(m_path.string(), doc);
                    m_timestamp = std::filesystem::last_write_time(m_path);

                    return;
                }
            }
            catch(const boost::interprocess::interprocess_exception& e)
            {
                throw lock_error(e.what());
            }
            catch(const std::exception& e)
            {
                throw file_error(e.what());
            }

            throw stale_error("config was modified since the last read");
        }

    public:

        webpier(const std::filesystem::path& home)
            : m_path(home / conf_file_name)
        {
            if (!std::filesystem::exists(m_path))
                throw usage_error("context is not exist");

            auto locker = home / lock_file_name;
            m_locker = boost::interprocess::file_lock(locker.string().c_str());

            load();
        }

        webpier(const std::filesystem::path& home, const std::string& host)
            : m_path(home / conf_file_name)
        {
            if (std::filesystem::exists(home) && !std::filesystem::is_empty(home))
                throw usage_error("bad context directory");

            m_config.host = host;

            try
            {
                {
                    std::filesystem::create_directories(home);

                    auto locker = home / lock_file_name;
                    std::ofstream(locker).close();

                    m_locker = boost::interprocess::file_lock(locker.string().c_str());
                }

                save();
            }
            catch (...)
            {
                std::filesystem::remove_all(home);
                throw;
            }
        }

        void get_config(config& info) const noexcept(true)
        {
            info = m_config; 
        }

        void set_config(const config& info) noexcept(false)
        {
            if (info.host != m_config.host)
                throw usage_error("renaming the hosting is forbidden");

            m_config = info;
            save();
        }

        std::string host() const noexcept(true)
        {
            return m_config.host;
        }

        std::filesystem::path path() const noexcept(true)
        {
            return m_path;
        }
    };

    class node
    {
        std::filesystem::path m_path;
        std::vector<service> m_units;
        boost::interprocess::file_lock m_locker;
        std::filesystem::file_time_type m_timestamp;

        void load() noexcept(false)
        {
            try
            {
                boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker, boost::interprocess::try_to_lock_type());

                boost::property_tree::ptree doc;
                boost::property_tree::read_json(m_path.string(), doc);

                boost::property_tree::ptree array;
                for (auto& item : doc.get_child("services", array))
                {
                    service unit;
                    unit.id = item.second.get<std::string>("id", "");
                    unit.peer = item.second.get<std::string>("peer", "");
                    unit.address = item.second.get<std::string>("address", "");
                    unit.gateway = item.second.get<std::string>("gateway", "");
                    unit.autostart = item.second.get<bool>("autostart", false);
                    unit.obscure = item.second.get<bool>("obscure", true);
                    unit.rendezvous.bootstrap = item.second.get<std::string>("rendezvous.dht.bootstrap", "");
                    unit.rendezvous.network = item.second.get<uint32_t>("rendezvous.dht.network", 0);
                    m_units.emplace_back(std::move(unit));
                }

                m_timestamp = std::filesystem::last_write_time(m_path);
            }
            catch(const boost::interprocess::interprocess_exception& e)
            {
                throw lock_error(e.what());
            }
            catch(const std::exception& e)
            {
                throw file_error(e.what());
            }
        }

        void save() noexcept(false)
        {
            try
            {
                boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker, boost::interprocess::try_to_lock_type());

                if (m_timestamp == std::filesystem::file_time_type() || std::filesystem::last_write_time(m_path) == m_timestamp)
                {
                    boost::property_tree::ptree array;
                    for (auto& unit : m_units)
                    {
                        boost::property_tree::ptree item;
                        item.put("id", unit.id);
                        item.put("peer", unit.peer);
                        item.put("address", unit.address);
                        item.put("gateway", unit.gateway);
                        item.put("autostart", unit.autostart);
                        item.put("obscure", unit.obscure);
                        item.put("rendezvous.dht.bootstrap", unit.rendezvous.bootstrap);
                        item.put("rendezvous.dht.network", unit.rendezvous.network);
                        array.push_back(std::make_pair("", item));
                    }

                    boost::property_tree::ptree doc;
                    doc.put_child("services", array);

                    boost::property_tree::write_json(m_path.string(), doc);
                    m_timestamp = std::filesystem::last_write_time(m_path);

                    return;
                }
            }
            catch(const boost::interprocess::interprocess_exception& e)
            {
                throw lock_error(e.what());
            }
            catch(const std::exception& e)
            {
                throw file_error(e.what());
            }

            throw stale_error("config was modified since the last read");
        }

    public:

        node(const std::filesystem::path& home, bool init = false)
            : m_path(home / conf_file_name)
        {
            if (init)
            {
                if (std::filesystem::exists(home) && !std::filesystem::is_empty(home))
                    throw usage_error("bad node directory");

                try
                {
                    {
                        std::filesystem::create_directories(home);

                        auto locker = home / lock_file_name;
                        std::ofstream(locker).close();

                        m_locker = boost::interprocess::file_lock(locker.string().c_str());
                        boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker, boost::interprocess::try_to_lock_type());
                        generate_x509_pair(home / cert_file_name, home / key_file_name, home.parent_path().filename().string() + "/" + home.filename().string());
                    }

                    save();
                }
                catch(...)
                {
                    std::filesystem::remove_all(home);
                    throw;
                }
            }
            else
            {
                if (!std::filesystem::exists(m_path))
                    throw usage_error("bad node directory");

                auto locker = home / lock_file_name;
                m_locker = boost::interprocess::file_lock(locker.string().c_str());

                load();
            }
        }

        node(const std::filesystem::path& home, const std::string& cert)
            : m_path(home / conf_file_name)
        {
            if (std::filesystem::exists(home) && !std::filesystem::is_empty(home))
                throw usage_error("bad node directory");

            try
            {
                {
                    std::filesystem::create_directories(home);

                    auto locker = home / lock_file_name;
                    std::ofstream(locker).close();

                    m_locker = boost::interprocess::file_lock(locker.string().c_str());
                    boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker, boost::interprocess::try_to_lock_type());
                    save_x509_cert(home / cert_file_name, cert);
                }
                save();
            }
            catch(...)
            {
                std::filesystem::remove_all(home);
                throw;
            }
        }

        void add(const service& info) noexcept(false)
        {
            auto iter = std::find_if(m_units.begin(), m_units.end(), [&info](const service& item)
            {
                return item.id == info.id;
            });

            if (iter != m_units.end())
                throw usage_error("such service already exists");

            m_units.push_back(info);
            save();
        }

        void del(const std::string& id) noexcept(false)
        {
            auto iter = std::remove_if(m_units.begin(), m_units.end(), [&id](const service& item)
            {
                return item.id == id;
            });

            if (iter != m_units.end())
            {
                m_units.erase(iter, m_units.end());
                save();
            }
        }

        bool get(const std::string& id, service& info) const noexcept(true)
        {
            auto iter = std::find_if(m_units.begin(), m_units.end(), [&id](const service& item)
            {
                return item.id == id;
            });

            if (iter == m_units.end())
                return false;

            info = *iter;
            return true;
        }

        void get(std::vector<service>& list) const noexcept(true)
        {
            std::copy(m_units.begin(), m_units.end(), std::back_inserter(list));
        }
    };

    class context_impl : public context
    {
        std::filesystem::path m_home;
        webpier m_config;
        std::map<std::string, node> m_context;

    public:

        context_impl(const std::filesystem::path& home)
            : m_home(home)
            , m_config(home)
        {
            for (auto const& owner : std::filesystem::directory_iterator(home / repo_dir_name))
            {
                if (!owner.is_directory())
                    continue;

                for (auto const& pier : std::filesystem::directory_iterator(owner.path()))
                {
                    if (!pier.is_directory())
                        continue;

                    if (!std::filesystem::exists(pier.path() / cert_file_name))
                        continue;

                    auto id = owner.path().filename().string() + "/" + pier.path().filename().string();
                    m_context.emplace(std::make_pair(id, node(pier)));
                }
            }

            auto iter = m_context.find(m_config.host());
            if (iter == m_context.end())
                throw usage_error("host node is not found");
        }

        context_impl(const std::filesystem::path& home, const std::string& host)
            : m_home(home)
            , m_config(home, host)
        {
            m_context.emplace(std::make_pair(host, node(home / repo_dir_name / host, true)));
        }

        void get_config(config& info) const noexcept(true) override
        {
            m_config.get_config(info);
        }

        void set_config(const config& info) noexcept(false) override
        {
            m_config.set_config(info);
        }

        void get_export_services(std::vector<service>& list) const noexcept(true) override
        {
            auto iter = m_context.find(m_config.host());
            if (iter != m_context.end())
                iter->second.get(list);
        }

        bool get_export_service(const std::string& id, service& info) const noexcept(true) override
        {
            auto iter = m_context.find(m_config.host());
            if (iter == m_context.end())
                return false;

            return iter->second.get(id, info);
        }

        void add_export_service(const service& info) noexcept(false) override
        {
            auto iter = m_context.find(m_config.host());
            if (iter == m_context.end())
                throw usage_error("host node is not found");

            iter->second.add(info);
        }

        void del_export_service(const std::string& id) noexcept(false) override
        {
            auto iter = m_context.find(m_config.host());
            if (iter == m_context.end())
                throw usage_error("host node is not found");

            iter->second.del(id);
        }

        void get_import_services(std::vector<service>& list) const noexcept(true) override
        {
            for (const auto& item : m_context)
            {
                if (item.first != m_config.host())
                    item.second.get(list);
            }
        }

        bool get_import_service(const std::string& peer, const std::string& id, service& info) const noexcept(true) override
        {
            auto iter = m_context.find(peer);
            if (iter == m_context.end())
                return false;

            return iter->second.get(id, info);
        }

        void add_import_service(const service& info) noexcept(false) override
        {
            auto iter = m_context.find(info.peer);
            if (iter == m_context.end())
                throw usage_error("peer node is not found");

            iter->second.add(info);
        }

        void del_import_service(const std::string& peer, const std::string& id) noexcept(false) override
        {
            auto iter = m_context.find(peer);
            if (iter == m_context.end())
                throw usage_error("peer node is not found");

            iter->second.del(id);
        }

        void get_peers(std::vector<std::string>& list) const noexcept(true) override
        {
            for (const auto& item : m_context)
            {
                if (item.first != m_config.host())
                    list.push_back(item.first);
            }
        }

        bool has_peer(const std::string& id) const noexcept(true) override
        {
            return m_context.find(id) != m_context.end();
        }

        void add_peer(const std::string& id, const std::string& cert) noexcept(false) override
        {
            auto home = m_config.path().parent_path() / repo_dir_name / id;

            if (std::filesystem::exists(home))
                throw usage_error("such node already exists");

            m_context.emplace(std::make_pair(id, node(home, cert)));
        }

        void del_peer(const std::string& id) noexcept(false) override
        {
            if (m_config.host() == id)
                throw usage_error("can't delete host node");

            m_context.erase(id);

            try
            {
                std::filesystem::remove_all(m_config.path().parent_path() / repo_dir_name / id);
            }
            catch(const std::exception& e)
            {
                throw file_error(e.what());
            }
        }

        std::string get_fingerprint(const std::string& id) const noexcept(false) override
        {
            return get_x509_public_sha1(m_config.path().parent_path() / repo_dir_name / id / cert_file_name);
        }

        std::string get_certificate(const std::string& id) const noexcept(false) override
        {
            return load_x509_cert(m_config.path().parent_path() / repo_dir_name / id / cert_file_name);
        }

        std::string get_home() const noexcept(true) override
        {
            return m_home.string();
        }

        std::string get_host() const noexcept(true) override
        {
            return m_config.host();
        }
    };

    std::shared_ptr<context> open_context(const std::string& dir, const std::string& host) noexcept(false)
    {
        auto home = std::filesystem::path(dir) / to_hexadecimal(host.data(), host.size());
        return std::filesystem::exists(home) ? std::make_shared<context_impl>(home) : std::make_shared<context_impl>(home, host);
    }
}
