#include "context.h"
#include <filesystem>
#include <boost/property_tree/json_parser.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace webpier
{
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

    class context_impl : public context
    {
        std::string m_dir;
        basic m_basic;
        std::vector<service> m_locals;
        std::vector<service> m_remotes;
        std::filesystem::file_time_type m_timestamp;

        enum flush_mode { flush_basic = 0x1, flush_local = 0x2, flush_remote = 0x4, flush_all = flush_basic|flush_local|flush_remote };

        void flush(int mode = flush_all)
        {
            try
            {
                boost::interprocess::file_lock lock(m_dir.c_str());
                boost::interprocess::scoped_lock<boost::interprocess::file_lock> scope(lock);

                if (std::filesystem::last_write_time(m_dir) == m_timestamp)
                {
                    if (mode & flush_basic)
                    {
                        boost::property_tree::ptree info;

                        info.put("host", m_basic.host);
                        info.put("report", m_basic.report);
                        info.put("daemon", m_basic.daemon);
                        info.put("tray", m_basic.tray);
                        info.put("traverse.stun", m_basic.traverse.stun);
                        info.put("traverse.hops", m_basic.traverse.hops);
                        info.put("dht.bootstrap", m_basic.rendezvous.bootstrap);
                        info.put("dht.network", m_basic.rendezvous.network);
                        info.put("emailer.smtp", m_basic.emailer.smtp);
                        info.put("emailer.imap", m_basic.emailer.imap);
                        info.put("emailer.login", m_basic.emailer.login);
                        info.put("emailer.password", m_basic.emailer.password);
                        info.put("emailer.cert", m_basic.emailer.cert);
                        info.put("emailer.key", m_basic.emailer.key);
                        info.put("emailer.ca", m_basic.emailer.ca);

                        boost::property_tree::write_json(m_dir + "/webpier.json", info);
                    }

                    if (mode & flush_local)
                    {
                        boost::property_tree::ptree list;

                        for (auto& info : m_locals)
                        {
                            boost::property_tree::ptree item;
                            item.put("id", info.id);
                            item.put("peer", info.peer);
                            item.put("service", info.mapping);
                            item.put("gateway", info.gateway);
                            item.put("autostart", info.autostart);
                            item.put("obscure", info.obscure);
                            if (!info.rendezvous.bootstrap.empty())
                            {
                                item.put("dht.bootstrap", info.rendezvous.bootstrap);
                                item.put("dht.network", info.rendezvous.network);
                            }
                            list.push_back(std::make_pair("", item));
                        }

                        boost::property_tree::write_json(m_dir + "/local.json", list);
                    }

                    if (mode & flush_remote)
                    {
                        boost::property_tree::ptree list;

                        for (auto& info : m_remotes)
                        {
                            boost::property_tree::ptree item;
                            item.put("id", info.id);
                            item.put("peer", info.peer);
                            item.put("service", info.mapping);
                            item.put("gateway", info.gateway);
                            item.put("autostart", info.autostart);
                            item.put("obscure", info.obscure);
                            if (!info.rendezvous.bootstrap.empty())
                            {
                                item.put("dht.bootstrap", info.rendezvous.bootstrap);
                                item.put("dht.network", info.rendezvous.network);
                            }
                            list.push_back(std::make_pair("", item));
                        }

                        boost::property_tree::write_json(m_dir + "/remote.json", list);
                    }

                    m_timestamp = std::filesystem::file_time_type::clock::now();
                    std::filesystem::last_write_time(m_dir, m_timestamp);
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

        context_impl(const std::string& dir)
            : m_dir(dir)
        {
            reload();
        }

        void reload() noexcept(false) override
        {
            try
            {
                boost::interprocess::file_lock lock(m_dir.c_str());
                boost::interprocess::scoped_lock<boost::interprocess::file_lock> scope(lock);

                boost::property_tree::ptree info;
                boost::property_tree::read_json(m_dir + "/webpier.json", info);

                m_basic.host = info.get<std::string>("host", "");
                m_basic.report = info.get<log>("report", log::info);
                m_basic.daemon = info.get<bool>("daemon", false);
                m_basic.tray = info.get<bool>("tray", true);
                m_basic.traverse.stun = info.get<std::string>("traverse.stun", "stun.ekiga.net");
                m_basic.traverse.hops = info.get<uint8_t>("traverse.hops", 7);
                m_basic.rendezvous.bootstrap = info.get<std::string>("dht.bootstrap", "bootstrap.jami.net:4222");
                m_basic.rendezvous.network = info.get<uint32_t>("dht.network", 0);
                m_basic.emailer.smtp = info.get<std::string>("emailer.smtp", "");
                m_basic.emailer.imap = info.get<std::string>("emailer.imap", "");
                m_basic.emailer.login = info.get<std::string>("emailer.login", "");
                m_basic.emailer.password = info.get<std::string>("emailer.password", "");
                m_basic.emailer.cert = info.get<std::string>("emailer.cert", "");
                m_basic.emailer.key = info.get<std::string>("emailer.key", "");
                m_basic.emailer.ca = info.get<std::string>("emailer.ca", "");

                boost::property_tree::ptree local;
                boost::property_tree::read_json(m_dir + "/local.json", info);
                for (auto& item : local)
                {
                    service loc;
                    loc.id = item.second.get<std::string>("id", "");
                    loc.peer = item.second.get<std::string>("peer", "");
                    loc.mapping = item.second.get<std::string>("mapping", "");
                    loc.gateway = item.second.get<std::string>("gateway", "");
                    loc.autostart = item.second.get<bool>("autostart", false);
                    loc.obscure = item.second.get<bool>("obscure", true);
                    loc.rendezvous.bootstrap = item.second.get<std::string>("dht.bootstrap", "bootstrap.jami.net:4222");
                    loc.rendezvous.network = item.second.get<uint32_t>("dht.network", 0);
                    m_locals.emplace_back(std::move(loc));
                }

                boost::property_tree::ptree remote;
                boost::property_tree::read_json(m_dir + "/remote.json", info);
                for (auto& item : remote)
                {
                    service rem;
                    rem.id = item.second.get<std::string>("id", "");
                    rem.peer = item.second.get<std::string>("peer", "");
                    rem.mapping = item.second.get<std::string>("service", "");
                    rem.gateway = item.second.get<std::string>("gateway", "");
                    rem.autostart = item.second.get<bool>("autostart", false);
                    rem.obscure = item.second.get<bool>("obscure", true);
                    rem.rendezvous.bootstrap = item.second.get<std::string>("dht.bootstrap", "bootstrap.jami.net:4222");
                    rem.rendezvous.network = item.second.get<uint32_t>("dht.network", 0);
                    m_remotes.emplace_back(std::move(rem));
                }

                m_timestamp = std::filesystem::last_write_time(m_dir);
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

        void get_basic(basic& out) const noexcept(true) override
        {
            out = m_basic; 
        }

        void set_basic(const basic& info) noexcept(true) override
        {
            int mode = flush_basic;
            if (m_basic.host != info.host)
            {
                m_remotes.clear();
                m_locals.clear();

                mode |= flush_remote|flush_local;
            }

            m_basic = info;
            flush(mode);
        }

        void get_local_services(std::vector<service>& out) const noexcept(true) override
        {
            out = m_locals;
        }

        void add_local_service(const service& info) noexcept(false) override
        {
            auto iter = std::find_if(m_locals.begin(), m_locals.end(), [&info](const service& item)
            {
                return item.id == info.id;
            });

            if (iter != m_locals.end())
                throw unique_error("such local service is already exist");

            m_locals.push_back(info);
            flush(flush_local);
        }

        void del_local_service(const std::string& id) noexcept(true) override
        {
            auto iter = std::remove_if(m_locals.begin(), m_locals.end(), [&id](const service& item)
            {
                return item.id == id;
            });

            if (iter != m_locals.end())
                m_locals.erase(iter, m_locals.end());

            flush(flush_local);
        }

        void get_remote_services(std::vector<service>& out) const noexcept(true) override
        {
            out = m_remotes;
        }

        void add_remote_service(const service& info) noexcept(false) override
        {
            auto iter = std::find_if(m_remotes.begin(), m_remotes.end(), [&info](const service& item)
            {
                return item.id == info.id && item.peer == info.peer;
            });

            if (iter != m_remotes.end())
                throw unique_error("such remote service is already exist");

            m_remotes.push_back(info);
            flush(flush_remote);
        }

        void del_remote_service(const std::string& peer, const std::string& id) noexcept(true) override
        {
            auto iter = std::remove_if(m_remotes.begin(), m_remotes.end(), [&id, &peer](const service& item)
            {
                return item.peer == peer && item.id == id;
            });

            if (iter != m_remotes.end())
                m_remotes.erase(iter, m_remotes.end());
            flush(flush_remote);
        }
    };

    std::shared_ptr<context> open_context(const std::string& dir)
    {
        return std::make_shared<context_impl>(dir);
    }
}
