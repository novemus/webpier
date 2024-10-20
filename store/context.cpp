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
    constexpr const char* repo_dir_name = "repo";

    std::ostream& operator<<(std::ostream& out, log level)
    {
        switch(level)
        {
            case log::fatal:
                return out << "FATAL";
            case log::error:
                return out << "ERROR";
            case log::warning:
                return out << "WARN";
            case log::info:
                return out << "INFO";
            case log::debug:
                return out << "DEBUG";
            case log::trace:
                return out << "TRACE";
            default:
                return out << "NONE";
        }
        return out;
    }

    std::istream& operator>>(std::istream& in, log& level)
    {
        std::string str;
        in >> str;

        if (str == "fatal" || str == "FATAL" || str == "1")
            level = log::fatal;
        else if (str == "error" || str == "ERROR" || str == "2")
            level = log::error;
        else if (str == "warning" || str == "WARN" || str == "3")
            level = log::warning;
        else if (str == "info" || str == "INFO" || str == "4")
            level = log::info;
        else if (str == "debug" || str == "DEBUG" || str == "5")
            level = log::debug;
        else if (str == "trace" || str == "TRACE" || str == "6")
            level = log::trace;
        else
            level = log::info;

        return in;
    }

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

                boost::property_tree::ptree info;
                boost::property_tree::read_json(m_path, info);

                m_config.host = info.get<std::string>("host");
                m_config.report = info.get<log>("report", log::info);
                m_config.daemon = info.get<bool>("daemon", false);
                m_config.tray = info.get<bool>("tray", true);
                m_config.traverse.stun = info.get<std::string>("nat.traverse.stun", default_stun_server);
                m_config.traverse.hops = info.get<uint8_t>("nat.traverse.hops", 7);
                m_config.rendezvous.bootstrap = info.get<std::string>("rendezvous.dht.bootstrap", default_dht_bootstrap);
                m_config.rendezvous.network = info.get<uint32_t>("rendezvous.dht.network", 0);
                m_config.emailer.smtp = info.get<std::string>("emailer.smtp", "");
                m_config.emailer.imap = info.get<std::string>("emailer.imap", "");
                m_config.emailer.login = info.get<std::string>("emailer.login", "");
                m_config.emailer.password = info.get<std::string>("emailer.password", "");
                m_config.emailer.cert = info.get<std::string>("emailer.cert", "");
                m_config.emailer.key = info.get<std::string>("emailer.key", "");
                m_config.emailer.ca = info.get<std::string>("emailer.ca", "");

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

                if (std::filesystem::last_write_time(m_path) == m_timestamp)
                {
                    boost::property_tree::ptree info;

                    info.put("host", m_config.host);
                    info.put("report", m_config.report);
                    info.put("daemon", m_config.daemon);
                    info.put("tray", m_config.tray);
                    info.put("nat.traverse.stun", m_config.traverse.stun);
                    info.put("nat.traverse.hops", m_config.traverse.hops);
                    info.put("rendezvous.dht.bootstrap", m_config.rendezvous.bootstrap);
                    info.put("rendezvous.dht.network", m_config.rendezvous.network);
                    info.put("emailer.smtp", m_config.emailer.smtp);
                    info.put("emailer.imap", m_config.emailer.imap);
                    info.put("emailer.login", m_config.emailer.login);
                    info.put("emailer.password", m_config.emailer.password);
                    info.put("emailer.cert", m_config.emailer.cert);
                    info.put("emailer.key", m_config.emailer.key);
                    info.put("emailer.ca", m_config.emailer.ca);

                    boost::property_tree::write_json(m_path, info);
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

            m_locker = boost::interprocess::file_lock(m_path.c_str());

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
                    std::ofstream(m_path).close();

                    m_locker = boost::interprocess::file_lock(m_path.c_str());
                    boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker, boost::interprocess::try_to_lock_type());
                    m_timestamp = std::filesystem::last_write_time(m_path);
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

    class subject
    {
        std::filesystem::path m_path;
        std::vector<service> m_listing;
        boost::interprocess::file_lock m_locker;
        std::filesystem::file_time_type m_timestamp;

        void load() noexcept(false)
        {
            try
            {
                boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker, boost::interprocess::try_to_lock_type());

                boost::property_tree::ptree info;
                boost::property_tree::read_json(m_path, info);

                for (auto& item : info)
                {
                    service unit;
                    unit.id = item.second.get<std::string>("id", "");
                    unit.peer = item.second.get<std::string>("peer", "");
                    unit.service = item.second.get<std::string>("service", "");
                    unit.gateway = item.second.get<std::string>("gateway", "");
                    unit.autostart = item.second.get<bool>("autostart", false);
                    unit.obscure = item.second.get<bool>("obscure", true);
                    unit.rendezvous.bootstrap = item.second.get<std::string>("rendezvous.dht.bootstrap", "");
                    unit.rendezvous.network = item.second.get<uint32_t>("rendezvous.dht.network", 0);
                    m_listing.emplace_back(std::move(unit));
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

                if (std::filesystem::last_write_time(m_path) == m_timestamp)
                {
                    boost::property_tree::ptree info;
                    for (auto& unit : m_listing)
                    {
                        boost::property_tree::ptree item;
                        item.put("id", unit.id);
                        item.put("peer", unit.peer);
                        item.put("service", unit.service);
                        item.put("gateway", unit.gateway);
                        item.put("autostart", unit.autostart);
                        item.put("obscure", unit.obscure);
                        item.put("rendezvous.dht.bootstrap", unit.rendezvous.bootstrap);
                        item.put("rendezvous.dht.network", unit.rendezvous.network);
                        info.push_back(std::make_pair("", item));
                    }

                    boost::property_tree::write_json(m_path, info);
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

        subject(const std::filesystem::path& home, bool init = false)
            : m_path(home / conf_file_name)
        {
            if (init)
            {
                if (std::filesystem::exists(home) && !std::filesystem::is_empty(home))
                    throw usage_error("bad subject directory");

                try
                {
                    {
                        std::filesystem::create_directories(home);
                        std::ofstream(m_path.string()).close();

                        m_locker = boost::interprocess::file_lock(m_path.c_str());
                        boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker, boost::interprocess::try_to_lock_type());
                        m_timestamp = std::filesystem::last_write_time(m_path);

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
                    throw usage_error("subject does not exist");

                m_locker = boost::interprocess::file_lock(m_path.c_str());

                load();
            }
        }

        subject(const std::filesystem::path& home, const std::string& cert)
            : m_path(home / conf_file_name)
        {
            if (std::filesystem::exists(home) && !std::filesystem::is_empty(home))
                throw usage_error("bad subject directory");

            try
            {
                {
                    std::filesystem::create_directories(home);
                    std::ofstream outfile(m_path.string());
                    outfile.close();

                    m_locker = boost::interprocess::file_lock(m_path.c_str());
                    boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker, boost::interprocess::try_to_lock_type());

                    m_timestamp = std::filesystem::last_write_time(m_path);
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
            auto iter = std::find_if(m_listing.begin(), m_listing.end(), [&info](const service& item)
            {
                return item.id == info.id;
            });

            if (iter != m_listing.end())
                throw usage_error("such service already exists");

            m_listing.push_back(info);
            save();
        }

        void del(const std::string& id) noexcept(false)
        {
            auto iter = std::remove_if(m_listing.begin(), m_listing.end(), [&id](const service& item)
            {
                return item.id == id;
            });

            if (iter != m_listing.end())
            {
                m_listing.erase(iter, m_listing.end());
                save();
            }
        }

        void get(std::vector<service>& list) const noexcept(true)
        {
            std::copy(m_listing.begin(), m_listing.end(), std::back_inserter(list));
        }
    };

    class context_impl : public context
    {
        webpier m_config;
        std::map<std::string, subject> m_context;

    public:

        context_impl(const std::filesystem::path& dir)
            : m_config(dir)
        {
            for (auto const& owner : std::filesystem::directory_iterator(dir / repo_dir_name))
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
                    m_context.emplace(std::make_pair(id, subject(pier)));
                }
            }

            auto iter = m_context.find(m_config.host());
            if (iter == m_context.end())
                throw usage_error("host context is not found");
        }

        context_impl(const std::filesystem::path& dir, const std::string& host)
            : m_config(dir, host)
        {
            m_context.emplace(std::make_pair(host, subject(dir / repo_dir_name / host, true)));
        }

        void get_config(config& info) const noexcept(true) override
        {
            m_config.get_config(info);
        }

        void set_config(const config& info) noexcept(false) override
        {
            m_config.set_config(info);
        }

        void get_local_services(std::vector<service>& list) const noexcept(true) override
        {
            auto iter = m_context.find(m_config.host());
            if (iter != m_context.end())
                iter->second.get(list);
        }

        void add_local_service(const service& info) noexcept(false) override
        {
            auto iter = m_context.find(m_config.host());
            if (iter == m_context.end())
                throw usage_error("host context is not found");

            iter->second.add(info);
        }

        void del_local_service(const std::string& id) noexcept(false) override
        {
            auto iter = m_context.find(m_config.host());
            if (iter == m_context.end())
                throw usage_error("host context is not found");

            iter->second.del(id);
        }

        void get_remote_services(std::vector<service>& list) const noexcept(true) override
        {
            for (const auto& item : m_context)
            {
                if (item.first != m_config.host())
                    item.second.get(list);
            }
        }

        void add_remote_service(const service& info) noexcept(false) override
        {
            auto iter = m_context.find(info.peer);
            if (iter == m_context.end())
                throw usage_error("peer context is not found");

            iter->second.add(info);
        }

        void del_remote_service(const std::string& peer, const std::string& id) noexcept(false) override
        {
            auto iter = m_context.find(peer);
            if (iter == m_context.end())
                throw usage_error("peer context is not found");

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

        void add_peer(const std::string& id, const std::string& cert) noexcept(false) override
        {
            auto home = m_config.path().parent_path() / repo_dir_name / id;

            if (std::filesystem::exists(home))
                throw usage_error("such peer already exists");

            m_context.emplace(std::make_pair(id, subject(home, cert)));
        }

        void del_peer(const std::string& id) noexcept(false) override
        {
            if (m_config.host() == id)
                throw usage_error("can't delete host");

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
    };

    std::shared_ptr<context> open_context(const std::string& dir, const std::string& host) noexcept(false)
    {
        auto home = std::filesystem::path(dir) / to_hexadecimal(host.data(), host.size());
        return std::filesystem::exists(home) ? std::make_shared<context_impl>(home) : std::make_shared<context_impl>(home, host);
    }
}
