#include "context.h"
#include "utils.h"
#include <map>
#include <filesystem>
#include <boost/property_tree/json_parser.hpp>

namespace webpier
{
    constexpr const char* cert_file_name = "cert.crt";
    constexpr const char* key_file_name = "private.key";
    constexpr const char* conf_file_name = "webpier.json";
    constexpr const char* lock_file_name = "lock";
    constexpr const char* repo_dir_name = "repo";

    class locker
    {
        std::string m_path;
        FILE* m_lock = nullptr;

    public:

        locker(const std::filesystem::path& file) : m_path(file.string()), m_lock(std::fopen(m_path.c_str(), "wx"))
        {
            if (!m_lock)
                throw file_error("can't acquire lock");
        }

        ~locker()
        {
            std::fclose(m_lock);
            std::remove(m_path.c_str());
        }
    };

    class master
    {
        config m_config;
        std::filesystem::path m_path;
        std::filesystem::file_time_type m_timestamp;

        void load() noexcept(false)
        {
            try
            {
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
                m_config.autostart = doc.get<bool>("autostart", false);

                m_timestamp = std::filesystem::last_write_time(m_path);
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
                    doc.put("autostart", m_config.autostart);

                    boost::property_tree::write_json(m_path.string(), doc);
                    m_timestamp = std::filesystem::last_write_time(m_path);

                    return;
                }
            }
            catch(const std::exception& e)
            {
                throw file_error(e.what());
            }

            throw stale_error("config was modified since the last read");
        }

    public:

        master(const std::filesystem::path& home)
            : m_path(home / conf_file_name)
        {
            if (!std::filesystem::exists(m_path))
                throw usage_error("context config does not exist");

            load();
        }

        master(const std::filesystem::path& home, const std::string& host)
            : m_path(home / conf_file_name)
        {
            if (std::filesystem::exists(home) && !std::filesystem::is_empty(home))
                throw usage_error("bad context directory");

            m_config.host = host;

            try
            {
                std::filesystem::create_directories(home);
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
                throw usage_error("renaming host node is forbidden");

            m_config = info;
            save();
        }

        std::string host() const noexcept(true)
        {
            return m_config.host;
        }
    };

    class node
    {
        std::filesystem::path m_path;
        std::vector<service> m_list;
        std::filesystem::file_time_type m_timestamp;

        void load() noexcept(false)
        {
            try
            {
                boost::property_tree::ptree doc;
                boost::property_tree::read_json(m_path.string(), doc);

                boost::property_tree::ptree array;
                for (auto& item : doc.get_child("services", array))
                {
                    service unit;
                    unit.name = item.second.get<std::string>("name", "");
                    unit.peer = item.second.get<std::string>("peer", "");
                    unit.address = item.second.get<std::string>("address", "");
                    unit.gateway = item.second.get<std::string>("gateway", "");
                    unit.autostart = item.second.get<bool>("autostart", false);
                    unit.obscure = item.second.get<bool>("obscure", true);
                    unit.rendezvous.bootstrap = item.second.get<std::string>("rendezvous.dht.bootstrap", "");
                    unit.rendezvous.network = item.second.get<uint32_t>("rendezvous.dht.network", 0);
                    m_list.emplace_back(std::move(unit));
                }

                m_timestamp = std::filesystem::last_write_time(m_path);
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
                if (m_timestamp == std::filesystem::file_time_type() || std::filesystem::last_write_time(m_path) == m_timestamp)
                {
                    boost::property_tree::ptree array;
                    for (auto& unit : m_list)
                    {
                        boost::property_tree::ptree item;
                        item.put("name", unit.name);
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
                    std::filesystem::create_directories(home);
                    generate_x509_pair(home / cert_file_name, home / key_file_name, home.parent_path().filename().string() + "/" + home.filename().string());
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
                std::filesystem::create_directories(home);
                save_x509_cert(home / cert_file_name, cert);
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
            auto iter = std::find_if(m_list.begin(), m_list.end(), [&info](const service& item)
            {
                return item.name == info.name;
            });

            if (iter != m_list.end())
                throw usage_error("service already exists");

            m_list.push_back(info);
            save();
        }

        void del(const std::string& name) noexcept(false)
        {
            auto iter = std::remove_if(m_list.begin(), m_list.end(), [&name](const service& item)
            {
                return item.name == name;
            });

            if (iter != m_list.end())
            {
                m_list.erase(iter, m_list.end());
                save();
            }
        }

        bool get(const std::string& name, service& info) const noexcept(true)
        {
            auto iter = std::find_if(m_list.begin(), m_list.end(), [&name](const service& item)
            {
                return item.name == name;
            });

            if (iter == m_list.end())
                return false;

            info = *iter;
            return true;
        }

        void get(std::vector<service>& list) const noexcept(true)
        {
            std::copy(m_list.begin(), m_list.end(), std::back_inserter(list));
        }
    };

    class context_impl : public context
    {
        std::filesystem::path m_home;
        master m_info;
        std::map<std::string, node> m_bundle;

    public:

        context_impl(const std::filesystem::path& home)
            : m_home(home)
            , m_info(home)
        {
            for (auto const& owner : std::filesystem::directory_iterator(home / repo_dir_name))
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
                    m_bundle.emplace(std::make_pair(id, node(child)));
                }
            }

            auto iter = m_bundle.find(m_info.host());
            if (iter == m_bundle.end())
                throw usage_error("host node is not found");
        }

        context_impl(const std::filesystem::path& home, const std::string& host)
            : m_home(home)
            , m_info(home, host)
        {
            m_bundle.emplace(std::make_pair(host, node(home / repo_dir_name / host, true)));
        }

        std::string get_home() const noexcept(true) override
        {
            return m_home;
        }

        void get_config(config& info) const noexcept(true) override
        {
            m_info.get_config(info);
        }

        void set_config(const config& info) noexcept(false) override
        {
            locker lock(m_home.parent_path() / lock_file_name);
            m_info.set_config(info);
        }

        void get_export_services(std::vector<service>& list) const noexcept(true) override
        {
            auto iter = m_bundle.find(m_info.host());
            if (iter != m_bundle.end())
                iter->second.get(list);
        }

        void add_export_service(const service& info) noexcept(false) override
        {
            auto iter = m_bundle.find(m_info.host());
            if (iter == m_bundle.end())
                throw usage_error("host node is not found");

            locker lock(m_home.parent_path() / lock_file_name);
            iter->second.add(info);
        }

        void del_export_service(const std::string& name) noexcept(false) override
        {
            auto iter = m_bundle.find(m_info.host());
            if (iter == m_bundle.end())
                throw usage_error("host node is not found");

            locker lock(m_home.parent_path() / lock_file_name);
            iter->second.del(name);
        }

        void get_import_services(std::vector<service>& list) const noexcept(true) override
        {
            for (const auto& item : m_bundle)
            {
                if (item.first != m_info.host())
                    item.second.get(list);
            }
        }

        void add_import_service(const service& info) noexcept(false) override
        {
            auto iter = m_bundle.find(info.peer);
            if (iter == m_bundle.end())
                throw usage_error("peer node is not found");

            locker lock(m_home.parent_path() / lock_file_name);
            iter->second.add(info);
        }

        void del_import_service(const std::string& peer, const std::string& id) noexcept(false) override
        {
            auto iter = m_bundle.find(peer);
            if (iter == m_bundle.end())
                throw usage_error("peer node is not found");

            locker lock(m_home.parent_path() / lock_file_name);
            iter->second.del(id);
        }

        void get_peers(std::vector<std::string>& list) const noexcept(true) override
        {
            for (const auto& item : m_bundle)
            {
                if (item.first != m_info.host())
                    list.push_back(item.first);
            }
        }

        void add_peer(const std::string& id, const std::string& cert) noexcept(false) override
        {
            locker lock(m_home.parent_path() / lock_file_name);

            auto home = m_home / repo_dir_name / id;

            if (std::filesystem::exists(home))
                throw usage_error("such peer node already exists");

            m_bundle.emplace(std::make_pair(id, node(home, cert)));
        }

        void del_peer(const std::string& id) noexcept(false) override
        {
            if (m_info.host() == id)
                throw usage_error("can't delete host node");

            locker lock(m_home.parent_path() / lock_file_name);
            m_bundle.erase(id);

            try
            {
                std::filesystem::remove_all(m_home / repo_dir_name / id);
            }
            catch(const std::exception& e)
            {
                throw file_error(e.what());
            }
        }

        std::string get_fingerprint(const std::string& id) const noexcept(false) override
        {
            locker lock(m_home.parent_path() / lock_file_name);
            return get_x509_public_sha1(m_home / repo_dir_name / id / cert_file_name);
        }

        std::string get_certificate(const std::string& id) const noexcept(false) override
        {
            locker lock(m_home.parent_path() / lock_file_name);
            return load_x509_cert(m_home / repo_dir_name / id / cert_file_name);
        }
    };

    std::shared_ptr<context> open_context(const std::string& link) noexcept(false)
    {
        auto root = std::filesystem::path(link).parent_path();
        locker lock(root / lock_file_name);

        if (!std::filesystem::exists(link) || !std::filesystem::is_symlink(link))
            throw usage_error("wrong symlink file: " + link);

        auto home = std::filesystem::read_symlink(link);
        if (!std::filesystem::is_directory(home) || root != home.parent_path())
            throw usage_error("wrong symlink: " + link);

        return std::make_shared<context_impl>(home);
    }

    std::shared_ptr<context> open_context(const std::string& link, const std::string& host, bool tidy) noexcept(false)
    {
        auto root = std::filesystem::path(link).parent_path();
        locker lock(root / lock_file_name);

        if (std::filesystem::exists(link))
        {
            if (!std::filesystem::is_symlink(link))
                throw usage_error("wrong symlink file: " + link);

            auto home = std::filesystem::read_symlink(link);
            if (tidy && std::filesystem::is_directory(home) && root == home.parent_path())
                std::filesystem::remove_all(home);

            std::filesystem::remove(link);
        }

        auto home = std::filesystem::path(root) / to_hexadecimal(host.data(), host.size());
        std::filesystem::create_directory_symlink(home, link);

        return std::filesystem::exists(home) ? std::make_shared<context_impl>(home) : std::make_shared<context_impl>(home, host);
    }
}
