#include <ui/context.h>
#include <ui/messagedialog.h>
#include <ui/startupdialog.h>
#include <store/context.h>
#include <store/utils.h>
#include <slipway/slipway.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/process.hpp>
#ifdef WIN32
#include <boost/process/windows.hpp>
#endif
#include <wx/stdpaths.h>
#include <wx/timer.h>

namespace WebPier
{
    namespace
    {
        wxString GetHome()
        {
            wxString home;
            if (!wxGetEnv("WEBPIER_HOME", &home))
                home = wxStandardPaths::Get().GetUserLocalDataDir();
            
            if (!wxFileName::Exists(home) && !wxFileName::Mkdir(home))
            {
                CMessageDialog dialog(nullptr, _("can't create home directory ") + home, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
                dialog.ShowModal();
                throw webpier::usage_error("no context");
            }

            return home;
        }

        std::shared_ptr<webpier::context> GetContext()
        {
            static std::shared_ptr<webpier::context> s_context;

            if (s_context)
                return s_context;

            auto home = GetHome().ToStdString(wxGet_wxConvUTF8());
            auto context = webpier::open_context(home);

            webpier::config config;
            context->get_config(config);

            if (config.pier.empty())
            {
                CStartupDialog dialog(nullptr);
                if (dialog.ShowModal() == wxID_OK)
                {
                    config.pier = dialog.GetIdentity().ToStdString(wxGet_wxConvUTF8());
                    config.repo = home + "/" + webpier::to_hexadecimal(config.pier.data(), config.pier.size());
                    config.log.folder = home + "/journal";
                }

                if (config.pier.empty())
                    throw webpier::usage_error("can't init context");

                context->set_config(config);
            }

            if (!config.autostart)
            {
#ifdef WIN32
                static boost::process::child s_daemon(webpier::find_exec("SLIPWAY_EXEC", SLIPWAY_EXEC), home, boost::process::windows::hide);
#else
                static boost::process::child s_daemon(webpier::find_exec("SLIPWAY_EXEC", SLIPWAY_EXEC), home);
#endif
                static wxTimer s_timer;

                s_timer.Bind(wxEVT_TIMER, [&](wxTimerEvent&)
                {
                    if (s_daemon.running())
                        s_daemon.detach();
                    else
                        s_daemon.join();
                    s_timer.Unlink();
                }, s_timer.GetId());

                s_timer.Start(5000, true);
            }

            return s_context = context;
        }

        std::shared_ptr<slipway::daemon> GetDaemon()
        {
            static std::shared_ptr<slipway::daemon> s_client;
            if (s_client)
                return s_client;

            auto home = GetHome().ToStdString(wxGet_wxConvUTF8());
            return s_client = slipway::create_client(home);
        }
    }

    namespace Context
    {
        bool IsEqual(ServicePtr lhs, ServicePtr rhs)
        {
            return lhs && rhs 
                && lhs->Name == rhs->Name
                && lhs->Pier == rhs->Pier
                && lhs->Address == rhs->Address
                && lhs->Gateway == rhs->Gateway
                && lhs->Rendezvous == rhs->Rendezvous
                && lhs->Autostart == rhs->Autostart
                && lhs->Obscure == rhs->Obscure;
        }

        template<bool isExport> class ServiceImpl : public Service
        {
            webpier::service m_origin;

        public:

            ServiceImpl(webpier::service origin = {})
                : m_origin(origin)
            {
                Revert();
            }

            void Store() noexcept(false) override
            {
                if (!IsDirty())
                    return;

                webpier::service actual {
                    Name.ToStdString(wxGet_wxConvUTF8()),
                    Pier.ToStdString(wxGet_wxConvUTF8()),
                    Address.ToStdString(wxGet_wxConvUTF8()),
                    Gateway.ToStdString(wxGet_wxConvUTF8()),
                    Rendezvous.ToStdString(wxGet_wxConvUTF8()),
                    Autostart,
                    Obscure
                };

                auto context = GetContext();
                if (IsExport())
                {
                    if (!m_origin.name.empty())
                        context->del_export_service(m_origin.name);
                    context->add_export_service(actual);
                }
                else
                {
                    if (!m_origin.name.empty() && !m_origin.pier.empty())
                        context->del_import_service(m_origin.pier, m_origin.name);
                    context->add_import_service(actual);
                }
                m_origin = actual;

                WebPier::Daemon::Handle handle{IsExport() ? Context::Pier() : Pier, Name};
                try
                {
                    WebPier::Daemon::Adjust(handle);
                }
                catch(const std::exception& ex)
                {
                    std::cout << "Can't adjust service " << handle.Pier << ":" << handle.Service;
                }
            }

            void Purge() noexcept(false) override
            {
                auto context = GetContext();
                if (IsExport())
                {
                    if (!m_origin.name.empty())
                        context->del_export_service(m_origin.name);
                }
                else
                {
                    if (!m_origin.name.empty() && !m_origin.pier.empty())
                        context->del_import_service(m_origin.pier, m_origin.name);
                }

                WebPier::Daemon::Handle handle{IsExport() ? Context::Pier() : Pier, Name};
                try
                {
                    WebPier::Daemon::Adjust(handle);
                }
                catch(const std::exception& ex)
                {
                    std::cout << "Can't adjust service " << handle.Pier << ":" << handle.Service;
                }

                m_origin = {};
            }

            void Revert() noexcept(true) override
            {
                Name = wxString::FromUTF8(m_origin.name);
                Pier = wxString::FromUTF8(m_origin.pier);
                Address = wxString::FromUTF8(m_origin.address);
                Gateway = wxString::FromUTF8(m_origin.gateway);
                Rendezvous = wxString::FromUTF8(m_origin.rendezvous);
                Autostart = m_origin.autostart;
                Obscure = m_origin.obscure;
            }

            void AddPier(const wxString& pier) noexcept(true) override
            {
                if (IsExport())
                {
                    auto arr = wxSplit(Pier, ' ');
                    if (arr.Index(pier) == wxNOT_FOUND)
                    {
                        arr.Add(pier);
                        Pier = wxJoin(arr, ' ');
                    }
                }
                else
                    Pier = pier;
            }

            void DelPier(const wxString& pier) noexcept(true) override
            {
                if (IsExport())
                {
                    auto arr = wxSplit(Pier, ' ');
                    auto ind = arr.Index(pier);
                    if (ind != wxNOT_FOUND)
                    {
                        arr.RemoveAt(ind);
                        Pier = wxJoin(arr, ' ');
                    }
                }
                else if (pier == Pier)
                    Pier.Clear();
            }

            bool HasPier(const wxString& pier) const noexcept(true) override
            {
                return IsExport() ? wxSplit(Pier, ' ').Index(pier) != wxNOT_FOUND : pier == Pier;
            }

            bool IsDirty() const noexcept(true) override
            {
                return Name.ToStdString(wxGet_wxConvUTF8()) != m_origin.name
                    || Pier.ToStdString(wxGet_wxConvUTF8()) != m_origin.pier
                    || Address.ToStdString(wxGet_wxConvUTF8()) != m_origin.address
                    || Gateway.ToStdString(wxGet_wxConvUTF8()) != m_origin.gateway
                    || Rendezvous.ToStdString(wxGet_wxConvUTF8()) != m_origin.rendezvous
                    || Autostart != m_origin.autostart
                    || Obscure != m_origin.obscure;
            }

            bool IsExport() const noexcept(true) override
            {
                return isExport;
            }

            bool IsImport() const noexcept(true) override
            {
                return !isExport;
            }
        };

        using ExportService = ServiceImpl<true>;
        using ImportService = ServiceImpl<false>;

        ServicePtr CreateExportService()
        {
            return ServicePtr(new ExportService());
        }

        ServicePtr CreateImportService()
        {
            return ServicePtr(new ImportService());
        }

        class ConfigImpl : public Config
        {
            webpier::config m_origin;

        public:

            ConfigImpl(webpier::config origin = {})
                : m_origin(origin)
            {
                Revert();
            }

            void Store() noexcept(false) override
            {
                webpier::config actual {
                    Pier.ToStdString(wxGet_wxConvUTF8()),
                    Repo.ToStdString(wxGet_wxConvUTF8()),
                    { LogFolder.ToStdString(wxGet_wxConvUTF8()), webpier::journal::severity(LogLevel) },
                    { StunServer.ToStdString(wxGet_wxConvUTF8()), PunchHops },
                    { DhtBootstrap.ToStdString(wxGet_wxConvUTF8()), DhtPort },
                    { 
                        SmtpServer.ToStdString(wxGet_wxConvUTF8()),
                        ImapServer.ToStdString(wxGet_wxConvUTF8()),
                        EmailLogin.ToStdString(wxGet_wxConvUTF8()),
                        EmailPassword.ToStdString(wxGet_wxConvUTF8()), 
                        EmailX509Cert.ToStdString(wxGet_wxConvUTF8()), 
                        EmailX509Key.ToStdString(wxGet_wxConvUTF8()), 
                        EmailX509Ca.ToStdString(wxGet_wxConvUTF8()) 
                    },
                    Autostart
                };

                GetContext()->set_config(actual);

                m_origin = actual;
            }

            void Revert() noexcept(true) override
            {
                Pier = wxString::FromUTF8(m_origin.pier);
                Repo = wxString::FromUTF8(m_origin.repo);
                LogFolder = wxString::FromUTF8(m_origin.log.folder);
                LogLevel = Logging(m_origin.log.level);
                StunServer = wxString::FromUTF8(m_origin.nat.stun);
                PunchHops = m_origin.nat.hops;
                DhtBootstrap = wxString::FromUTF8(m_origin.dht.bootstrap);
                DhtPort = m_origin.dht.port;
                SmtpServer = wxString::FromUTF8(m_origin.email.smtp);
                ImapServer = wxString::FromUTF8(m_origin.email.imap);
                EmailLogin = wxString::FromUTF8(m_origin.email.login);
                EmailPassword = wxString::FromUTF8(m_origin.email.password);
                EmailX509Cert = wxString::FromUTF8(m_origin.email.cert);
                EmailX509Key = wxString::FromUTF8(m_origin.email.key);
                EmailX509Ca = wxString::FromUTF8(m_origin.email.ca);
                Autostart = m_origin.autostart;
            }
        };

        wxString Pier() noexcept(false)
        {
            return wxString::FromUTF8(GetContext()->pier());
        }

        ConfigPtr GetConfig() noexcept(false)
        {
            webpier::config config;
            GetContext()->get_config(config);
            return ConfigPtr(new ConfigImpl(config));
        }

        ServiceList GetExportServices() noexcept(false)
        {
            ServiceList collection;
            std::vector<webpier::service> list;
            GetContext()->get_export_services(list);
            for (const auto& item : list)
            {
                ServicePtr ptr(new ExportService(item));
                collection[wxUIntPtr(ptr.get())] = ptr;
            }
            return collection;
        }

        ServiceList GetImportServices() noexcept(false)
        {
            ServiceList collection;
            std::vector<webpier::service> list;
            GetContext()->get_import_services(list);
            for (const auto& item : list)
            {
                ServicePtr ptr(new ImportService(item));
                collection[wxUIntPtr(ptr.get())] = ptr;
            }

            return collection;
        }

        wxArrayString GetPiers() noexcept(false)
        {
            wxArrayString array;
            std::vector<std::string> list;
            GetContext()->get_piers(list);
            for (const auto& item : list)
                array.Add(wxString::FromUTF8(item));
            return array;
        }

        bool IsUselessPier(const wxString& id) noexcept(false)
        {
            auto isUsedForRemote = [&]()
            {
                std::vector<webpier::service> list;
                GetContext()->get_import_services(list);
                auto iter = std::find_if(list.begin(), list.end(), [pier = id.ToStdString(wxGet_wxConvUTF8())](const auto& item)
                {
                    return item.pier == pier;
                });
                return iter != list.end();
            };

            auto isUsedForLocal = [&]()
            {
                std::vector<webpier::service> list;
                GetContext()->get_export_services(list);
                auto iter = std::find_if(list.begin(), list.end(), [pier = id.ToStdString(wxGet_wxConvUTF8())](const auto& item)
                {
                    return item.pier.find(pier) != std::string::npos;
                });
                return iter != list.end();
            };

            return !isUsedForRemote() && !isUsedForLocal();
        }

        bool IsUnknownPier(const wxString& id) noexcept(false)
        {
            return GetPiers().Index(id) == wxNOT_FOUND;
        }

        void AddPier(const wxString& id, const wxString& cert) noexcept(false)
        {
            GetContext()->add_pier(id.ToStdString(wxGet_wxConvUTF8()), cert.ToStdString(wxGet_wxConvUTF8()));
        }

        void DelPier(const wxString& id) noexcept(false)
        {
            GetContext()->del_pier(id.ToStdString(wxGet_wxConvUTF8()));
        }

        wxString GetCertificate(const wxString& id) noexcept(false)
        {
            return GetContext()->get_certificate(id.ToStdString(wxGet_wxConvUTF8()));
        }

        wxString GetFingerprint(const wxString& id) noexcept(false)
        {
            return GetContext()->get_fingerprint(id.ToStdString(wxGet_wxConvUTF8()));
        }

        void WriteOffer(const wxString& file, const Offer& offer) noexcept(false)
        {
            boost::property_tree::ptree doc;
            doc.put("pier", offer.Pier.ToStdString(wxGet_wxConvUTF8()));
            doc.put("certificate", offer.Certificate.ToStdString(wxGet_wxConvUTF8()));

            boost::property_tree::ptree array;
            for (const auto& pair : offer.Services)
            {
                boost::property_tree::ptree item;
                item.put("name", pair.second->Name.ToStdString(wxGet_wxConvUTF8()));
                item.put("obscure", pair.second->Obscure);
                item.put("rendezvous", pair.second->Rendezvous.ToStdString(wxGet_wxConvUTF8()));
                array.push_back(std::make_pair("", item));
            }
            doc.put_child("services", array);

            boost::property_tree::write_json(file.ToStdString(wxGet_wxConvUTF8()), doc);
        }

        void ReadOffer(const wxString& file, Offer& offer) noexcept(false)
        {
            boost::property_tree::ptree doc;
            boost::property_tree::read_json(file.ToStdString(wxGet_wxConvUTF8()), doc);

            offer.Pier = wxString::FromUTF8(doc.get<std::string>("pier", ""));
            offer.Certificate = wxString::FromUTF8(doc.get<std::string>("certificate", ""));

            boost::property_tree::ptree array;
            for (auto& item : doc.get_child("services", array))
            {
                ServicePtr service(new ImportService());
                service->Name = wxString::FromUTF8(item.second.get<std::string>("name"));
                service->Pier = offer.Pier;
                service->Obscure = item.second.get<bool>("obscure");
                service->Rendezvous = wxString::FromUTF8(item.second.get<std::string>("rendezvous", ""));
                offer.Services[wxUIntPtr(service.get())] = service;
            }
        }
    }

    namespace Daemon
    {
        Health Convert(const slipway::health& val)
        {
            return Health { Handle {val.pier, val.service}, static_cast<Health::Status>(val.state) };
        }

        Report Convert(const slipway::report& val)
        {
            Report ret;
            ret.Pier = val.pier;
            ret.Service = val.service;
            ret.State = static_cast<Health::Status>(val.state);

            for (const auto& item : val.tunnels)
                ret.Tunnels.push_back(Report::Tunnel { item.pier, item.pid });
            
            return ret;
        }

        slipway::handle Convert(const Handle& val)
        {
            return slipway::handle{ val.Pier.ToStdString(wxGet_wxConvUTF8()), val.Service.ToStdString(wxGet_wxConvUTF8()) };
        }

        void Unplug(const Handle& handle) noexcept(false)
        {
            auto client = GetDaemon();
            client->unplug(slipway::handle{ handle.Pier.ToStdString(wxGet_wxConvUTF8()), handle.Service.ToStdString(wxGet_wxConvUTF8()) });
        }

        void Unplug() noexcept(false)
        {
            auto client = GetDaemon();
            client->unplug();
        }

        void Engage(const Handle& handle) noexcept(false)
        {
            auto client = GetDaemon();
            client->engage(Convert(handle));
        }

        void Adjust(const Handle& handle) noexcept(false)
        {
            auto client = GetDaemon();
            client->adjust(Convert(handle));
        }

        void Engage() noexcept(false)
        {
            auto client = GetDaemon();
            client->engage();
        }

        void Adjust() noexcept(false)
        {
            auto client = GetDaemon();
            client->adjust();
        }

        Health Status(const Handle& handle) noexcept(false)
        {
            auto client = GetDaemon();
            slipway::health result;
            client->status(Convert(handle), result);
            return Convert(result);
        }

        wxVector<Health> Status() noexcept(false)
        {
            auto client = GetDaemon();
            std::vector<slipway::health> result;
            client->status(result);

            wxVector<Health> ret;
            for (const auto& item : result)
                ret.push_back(Convert(item));

            return ret;
        }

        Report Review(const Handle& handle) noexcept(false)
        {
            auto client = GetDaemon();
            slipway::report result;
            client->review(slipway::handle{ handle.Pier.ToStdString(wxGet_wxConvUTF8()), handle.Service.ToStdString(wxGet_wxConvUTF8()) }, result);
            return Convert(result);
        }

        wxVector<Report> Review() noexcept(false)
        {
            auto client = GetDaemon();
            std::vector<slipway::report> result;
            client->review(result);

            wxVector<Report> ret;
            for (const auto& item : result)
                ret.push_back(Convert(item));

            return ret;
        }
    }
}
