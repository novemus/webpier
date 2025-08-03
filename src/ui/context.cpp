#include <ui/context.h>
#include <ui/messagedialog.h>
#include <ui/startupdialog.h>
#include <store/context.h>
#include <store/utils.h>
#include <backend/client.h>
#include <plexus/plexus.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/scope_exit.hpp>
#include <wx/stdpaths.h>
#include <wx/timer.h>
#include <filesystem>

#include <boost/version.hpp>
#if BOOST_VERSION >= 108800
    #include <boost/process/v1/child.hpp>
    #ifdef WIN32
        #include <boost/process/v1/windows.hpp>
    #endif
    #define boost_process boost::process::v1
#else
    #include <boost/process.hpp>
    #ifdef WIN32
        #include <boost/process/windows.hpp>
    #endif
    #define boost_process boost::process
#endif

namespace WebPier
{
    namespace
    {
        constexpr const char* stun_server_default_port = "3478";
        constexpr const char* stun_client_default_port = "0";

        std::shared_ptr<webpier::context> g_context;
        std::shared_ptr<slipway::client> g_backend;

        std::string GetHome()
        {
            wxString home;
            if (!wxGetEnv("WEBPIER_HOME", &home))
                home = wxStandardPaths::Get().GetUserLocalDataDir();

            if (!wxFileName::Exists(home) && !wxFileName::Mkdir(home))
                throw webpier::usage_error("Wrong home path");

            return home.ToStdString();
        }

        void InitContext()
        {
            auto home = GetHome();

            g_context = webpier::open_context(home);

            webpier::config config;
            g_context->get_config(config);

            if (config.pier.empty())
            {
                CStartupDialog dialog(nullptr);
                if (dialog.ShowModal() == wxID_OK)
                {
                    config.pier = dialog.GetIdentity().ToStdString();
                    config.repo = home + "/" + webpier::to_hexadecimal(config.pier.data(), config.pier.size());
                    config.log.folder = home + "/journal";
                }

                if (config.pier.empty())
                    throw webpier::usage_error("No pier identity");

                g_context->set_config(config);
            }

#ifdef WIN32
            static boost_process::child s_server(webpier::get_module_path(webpier::slipway_module).string(), home, boost_process::windows::hide);
#else
            static boost_process::child s_server(webpier::get_module_path(webpier::slipway_module).string(), home);
#endif
            if (!s_server.running())
            {
                s_server.join();
                throw webpier::usage_error("Can't start backend");
            }

            static wxTimer s_timer;

            s_timer.Bind(wxEVT_TIMER, [&](wxTimerEvent&)
            {
                if (s_server.running())
                    s_server.detach();
                else
                    s_server.join();
                s_timer.Unlink();
            }, s_timer.GetId());

            s_timer.Start(5000, true);

            g_backend = slipway::connect_backend(g_context->home());
            g_backend->adjust();
        }
    }

    bool Init()
    {
        try
        {
            if (webpier::get_module_path(webpier::webpier_module) != webpier::get_absolute_path(wxStandardPaths::Get().GetExecutablePath().ToStdString()))
                throw std::runtime_error(_("Wrong module path"));

            InitContext();

            std::cout << "The WebPier is launched for " + g_context->pier() << " at " << g_context->home() << std::endl;

            return true;
        }
        catch (const std::exception& ex)
        {
            CMessageDialog dialog(nullptr, _("Can't start the WebPier. ") + ex.what(), wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
            dialog.ShowModal();
        }
        return false;
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

        class ServiceImpl : public Service
        {
            webpier::service m_origin;

        public:

            ServiceImpl(webpier::service origin)
                : m_origin(origin)
            {
                Revert();
            }

            void Store() noexcept(false) override
            {
                if (!IsDirty())
                    return;

                webpier::service actual {
                    Local,
                    Name.ToStdString(),
                    Pier.ToStdString(),
                    Address.ToStdString(),
                    Gateway.ToStdString(),
                    Rendezvous.ToStdString(),
                    Autostart,
                    Obscure
                };

                if (Local)
                {
                    if (!m_origin.name.empty())
                        g_context->del_export_service(m_origin.name);
                    g_context->add_export_service(actual);
                }
                else
                {
                    if (!m_origin.name.empty() && !m_origin.pier.empty())
                        g_context->del_import_service(m_origin.pier, m_origin.name);
                    g_context->add_import_service(actual);
                }
                m_origin = actual;

                WebPier::Backend::Handle handle{Local ? Context::Pier() : Pier, Name};
                try
                {
                    WebPier::Backend::Adjust(handle);
                }
                catch(const std::exception& ex)
                {
                    std::cerr << "Can't adjust service " << handle.Pier << ":" << handle.Service << std::endl;
                }
            }

            void Purge() noexcept(false) override
            {
                if (Local)
                {
                    if (!m_origin.name.empty())
                        g_context->del_export_service(m_origin.name);
                }
                else
                {
                    if (!m_origin.name.empty() && !m_origin.pier.empty())
                        g_context->del_import_service(m_origin.pier, m_origin.name);
                }

                WebPier::Backend::Handle handle{Local ? Context::Pier() : Pier, Name};
                try
                {
                    WebPier::Backend::Adjust(handle);
                }
                catch(const std::exception& ex)
                {
                    std::cerr << "Can't adjust service " << handle.Pier << ":" << handle.Service << std::endl;
                }

                m_origin = {};
            }

            void Revert() noexcept(true) override
            {
                Local = m_origin.local;
                Name = m_origin.name;
                Pier = m_origin.pier;
                Address = m_origin.address;
                Gateway = m_origin.gateway;
                Rendezvous = m_origin.rendezvous;
                Autostart = m_origin.autostart;
                Obscure = m_origin.obscure;
            }

            void AddPier(const wxString& pier) noexcept(true) override
            {
                if (Local)
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
                if (Local)
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
                return Local ? wxSplit(Pier, ' ').Index(pier) != wxNOT_FOUND : pier == Pier;
            }

            bool IsDirty() const noexcept(true) override
            {
                return Name.ToStdString() != m_origin.name
                    || Pier.ToStdString() != m_origin.pier
                    || Address.ToStdString() != m_origin.address
                    || Gateway.ToStdString() != m_origin.gateway
                    || Rendezvous.ToStdString() != m_origin.rendezvous
                    || Autostart != m_origin.autostart
                    || Obscure != m_origin.obscure;
            }
        };

        ServicePtr CreateExportService()
        {
            return ServicePtr(new ServiceImpl( webpier::service {true}));
        }

        ServicePtr CreateImportService()
        {
            return ServicePtr(new ServiceImpl( webpier::service {false}));
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
                    Pier.ToStdString(),
                    Repo.ToStdString(),
                    { LogFolder.ToStdString(), webpier::journal::severity(LogLevel) },
                    { StunServer.ToStdString(), PunchHops },
                    { DhtBootstrap.ToStdString(), DhtPort },
                    { 
                        SmtpServer.ToStdString(),
                        ImapServer.ToStdString(),
                        EmailLogin.ToStdString(),
                        EmailPassword.ToStdString(), 
                        EmailX509Cert.ToStdString(), 
                        EmailX509Key.ToStdString(), 
                        EmailX509Ca.ToStdString() 
                    }
                };

                g_context->set_config(actual);

                bool reboot = actual.pier != m_origin.pier;
                m_origin = actual;

                if (reboot)
                {
                    WebPier::Backend::Unplug();
                    WebPier::Backend::Engage();
                }
            }

            void Revert() noexcept(true) override
            {
                Pier = m_origin.pier;
                Repo = m_origin.repo;
                LogFolder = m_origin.log.folder;
                LogLevel = Logging(m_origin.log.level);
                StunServer = m_origin.nat.stun;
                PunchHops = m_origin.nat.hops;
                DhtBootstrap = m_origin.dht.bootstrap;
                DhtPort = m_origin.dht.port;
                SmtpServer = m_origin.email.smtp;
                ImapServer = m_origin.email.imap;
                EmailLogin = m_origin.email.login;
                EmailPassword = m_origin.email.password;
                EmailX509Cert = m_origin.email.cert;
                EmailX509Key = m_origin.email.key;
                EmailX509Ca = m_origin.email.ca;
            }
        };

        wxString Pier() noexcept(false)
        {
            return g_context->pier();
        }

        ConfigPtr GetConfig() noexcept(false)
        {
            webpier::config config;
            g_context->get_config(config);
            return ConfigPtr(new ConfigImpl(config));
        }

        ServiceList GetExportServices() noexcept(false)
        {
            ServiceList collection;
            std::vector<webpier::service> list;
            g_context->get_export_services(list);
            for (const auto& item : list)
            {
                ServicePtr ptr(new ServiceImpl(item));
                collection[wxUIntPtr(ptr.get())] = ptr;
            }
            return collection;
        }

        ServiceList GetImportServices() noexcept(false)
        {
            ServiceList collection;
            std::vector<webpier::service> list;
            g_context->get_import_services(list);
            for (const auto& item : list)
            {
                ServicePtr ptr(new ServiceImpl(item));
                collection[wxUIntPtr(ptr.get())] = ptr;
            }

            return collection;
        }

        wxArrayString GetPiers() noexcept(false)
        {
            wxArrayString array;
            std::vector<std::string> list;
            g_context->get_piers(list);
            for (const auto& item : list)
                array.Add(item);
            return array;
        }

        bool IsUselessPier(const wxString& id) noexcept(false)
        {
            auto isUsedForRemote = [&]()
            {
                std::vector<webpier::service> list;
                g_context->get_import_services(list);
                auto iter = std::find_if(list.begin(), list.end(), [pier = id.ToStdString()](const auto& item)
                {
                    return item.pier == pier;
                });
                return iter != list.end();
            };

            auto isUsedForLocal = [&]()
            {
                std::vector<webpier::service> list;
                g_context->get_export_services(list);
                auto iter = std::find_if(list.begin(), list.end(), [pier = id.ToStdString()](const auto& item)
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
            g_context->add_pier(id.ToStdString(), cert.ToStdString());
        }

        void DelPier(const wxString& id) noexcept(false)
        {
            g_context->del_pier(id.ToStdString());
        }

        wxString GetCertificate(const wxString& id) noexcept(false)
        {
            return g_context->get_certificate(id.ToStdString());
        }

        wxString GetFingerprint(const wxString& id) noexcept(false)
        {
            return g_context->get_fingerprint(id.ToStdString());
        }

        void WriteOffer(const wxString& file, const Offer& offer) noexcept(false)
        {
            boost::property_tree::ptree doc;
            doc.put("pier", webpier::locale_to_utf8(offer.Pier.ToStdString()));
            doc.put("certificate", webpier::locale_to_utf8(offer.Certificate.ToStdString()));

            boost::property_tree::ptree array;
            for (const auto& pair : offer.Services)
            {
                boost::property_tree::ptree item;
                item.put("name", webpier::locale_to_utf8(pair.second->Name.ToStdString()));
                item.put("obscure", pair.second->Obscure);
                item.put("rendezvous", webpier::locale_to_utf8(pair.second->Rendezvous.ToStdString()));
                array.push_back(std::make_pair("", item));
            }
            doc.put_child("services", array);

            boost::property_tree::write_json(file.ToStdString(), doc);
        }

        void ReadOffer(const wxString& file, Offer& offer) noexcept(false)
        {
            boost::property_tree::ptree doc;
            boost::property_tree::read_json(file.ToStdString(), doc);

            offer.Pier = webpier::utf8_to_locale(doc.get<std::string>("pier", ""));
            offer.Certificate = webpier::utf8_to_locale(doc.get<std::string>("certificate", ""));

            boost::property_tree::ptree array;
            for (auto& item : doc.get_child("services", array))
            {
                ServicePtr service = CreateImportService();
                service->Name = webpier::utf8_to_locale(item.second.get<std::string>("name"));
                service->Pier = offer.Pier;
                service->Obscure = item.second.get<bool>("obscure");
                service->Rendezvous = webpier::utf8_to_locale(item.second.get<std::string>("rendezvous", ""));
                offer.Services[wxUIntPtr(service.get())] = service;
            }
        }
    }

    namespace Backend
    {
        Health Convert(const slipway::health& val)
        {
            return Health { Handle {val.pier, val.service}, static_cast<Health::Status>(val.state), val.message };
        }

        Report Convert(const slipway::report& val)
        {
            Report ret;
            ret.Pier = val.pier;
            ret.Service = val.service;
            ret.State = static_cast<Health::Status>(val.state);
            ret.Message = val.message;

            for (const auto& item : val.tunnels)
                ret.Tunnels.push_back(Report::Tunnel { item.pier, item.pid });
            
            return ret;
        }

        slipway::handle Convert(const Handle& val)
        {
            return slipway::handle{ val.Pier.ToStdString(), val.Service.ToStdString() };
        }

        void AssignAutostart()
        {
            webpier::assign_autostart(webpier::get_module_path(webpier::slipway_module), "\"" + g_context->home() + "\" daemon");
        }

        void RevokeAutostart()
        {
            webpier::revoke_autostart(webpier::get_module_path(webpier::slipway_module), "\"" + g_context->home() + "\" daemon");
        }

        bool VerifyAutostart()
        {
            try
            {
                return webpier::verify_autostart(webpier::get_module_path(webpier::slipway_module), "\"" + g_context->home() + "\" daemon");
            }
            catch (const std::exception& ex) 
            {
                std::cerr << "Can't verify the backend startup status" << std::endl;
            }
            return false;
        }

        void Unplug(const Handle& handle) noexcept(false)
        {
            g_backend->unplug(slipway::handle{ handle.Pier.ToStdString(), handle.Service.ToStdString() });
        }

        void Unplug() noexcept(false)
        {
            g_backend->unplug();
        }

        void Engage(const Handle& handle) noexcept(false)
        {
            g_backend->engage(Convert(handle));
        }

        void Adjust(const Handle& handle) noexcept(false)
        {
            g_backend->adjust(Convert(handle));
        }

        void Engage() noexcept(false)
        {
            g_backend->engage();
        }

        void Adjust() noexcept(false)
        {
            g_backend->adjust();
        }

        Health Status(const Handle& handle) noexcept(false)
        {
            slipway::health result;
            g_backend->status(Convert(handle), result);
            return Convert(result);
        }

        wxVector<Health> Status() noexcept(false)
        {
            std::vector<slipway::health> result;
            g_backend->status(result);

            wxVector<Health> ret;
            for (const auto& item : result)
                ret.push_back(Convert(item));

            return ret;
        }

        Report Review(const Handle& handle) noexcept(false)
        {
            slipway::report result;
            g_backend->review(slipway::handle{ handle.Pier.ToStdString(), handle.Service.ToStdString() }, result);
            return Convert(result);
        }

        wxVector<Report> Review() noexcept(false)
        {
            std::vector<slipway::report> result;
            g_backend->review(result);

            wxVector<Report> ret;
            for (const auto& item : result)
                ret.push_back(Convert(item));

            return ret;
        }
    }

    namespace Utils
    {
        void ExploreNat(const wxString& bind, const wxString& stun, const std::function<void(const NatState&)>& callback) noexcept(true)
        {
            std::thread([=]()
            {
                try
                {
                    boost::asio::io_context io;

                    plexus::explore_network(io,
                        webpier::make_udp_endpoint(webpier::locale_to_utf8(bind.ToStdString()), webpier::stun_client_default_port),
                        webpier::make_udp_endpoint(webpier::locale_to_utf8(stun.ToStdString()), webpier::stun_server_default_port), 
                        [callback](const plexus::traverse& res)
                        {
                            callback(NatState {
                                wxEmptyString,
                                res.traits.nat,
                                res.traits.hairpin,
                                res.traits.random_port,
                                res.traits.variable_address,
                                static_cast<NatState::Binding>(res.traits.mapping),
                                static_cast<NatState::Binding>(res.traits.filtering),
                                wxString(webpier::utf8_to_locale(res.inner_endpoint.address().to_string() + ":" + std::to_string(res.inner_endpoint.port()))),
                                wxString(webpier::utf8_to_locale(res.outer_endpoint.address().to_string() + ":" + std::to_string(res.outer_endpoint.port())))
                            });
                        },
                        [callback](const std::string& error)
                        {
                            callback(NatState { error });
                        });
                    io.run();
                }
                catch (const std::exception& ex)
                {
                    callback(NatState { ex.what() });
                }
            }).detach();
        }

        void CheckRendezvous(const plexus::rendezvous& receiver, const plexus::rendezvous& forwarder, const std::function<void(const wxString&)>& callback) noexcept(true)
        {
            std::thread([=]()
            {
                try
                {
                    webpier::config config;
                    g_context->get_config(config);

                    plexus::identity host { config.pier.substr(0, config.pier.find('/')), config.pier.substr(config.pier.find('/') + 1) };

                    boost::asio::io_context io;
                    plexus::receive_advent(io, receiver, "webpier", config.repo, host, host,
                        [&io, callback](const plexus::identity&, const plexus::identity&)
                        {
                            io.stop();
                            callback(wxEmptyString);
                        },
                        [&io, callback](const plexus::identity&, const plexus::identity&, const std::string& error)
                        {
                            io.stop();
                            callback(error);
                        });

                    plexus::forward_advent(io, forwarder, "webpier", config.repo, host, host,
                        [](const plexus::identity&, const plexus::identity&)
                        {
                        },
                        [&io, callback](const plexus::identity&, const plexus::identity&, const std::string& error)
                        {
                            io.stop();
                            callback(error);
                        });

                    boost::asio::deadline_timer timer(io);

                    timer.expires_from_now(boost::posix_time::seconds(60));
                    timer.async_wait([&](const boost::system::error_code& error)
                    {
                        io.stop();
                        callback("timeout");
                    });

                    io.run();
                }
                catch (const std::exception& ex)
                {
                    callback(ex.what());
                }
            }).detach();
        }

        void CheckDhtRendezvous(const wxString& bootstrap, wxUint32 network, wxUint16 port, const std::function<void(const wxString&)>& callback) noexcept(true)
        {
            try
            {
                plexus::rendezvous receiver {
                    plexus::dhtnode {
                        webpier::locale_to_utf8(bootstrap.ToStdString()),
                        port,
                        network
                    } 
                };

                std::srand(std::time(nullptr));

                plexus::rendezvous forwarder {
                    plexus::dhtnode {
                        webpier::locale_to_utf8(bootstrap.ToStdString()),
                        uint16_t(49152u + std::rand() % 16383u),
                        network
                    } 
                };

                CheckRendezvous(receiver, forwarder, callback);
            }
            catch (const std::exception& ex)
            {
                callback(ex.what());
            }
        }

        void CheckEmailRendezvous(const wxString& smtp, const wxString& imap, const wxString& login, const wxString& password, const wxString& cert, const wxString& key, const wxString& ca, const std::function<void(const wxString&)>& callback) noexcept(true)
        {
            try
            {
                plexus::rendezvous mediator {
                    plexus::emailer { 
                        webpier::make_tcp_endpoint(webpier::locale_to_utf8(smtp.ToStdString()), webpier::smtp_server_default_port),
                        webpier::make_tcp_endpoint(webpier::locale_to_utf8(imap.ToStdString()), webpier::imap_server_default_port),
                        webpier::locale_to_utf8(login.ToStdString()),
                        webpier::locale_to_utf8(password.ToStdString()),
                        webpier::locale_to_utf8(cert.ToStdString()),
                        webpier::locale_to_utf8(key.ToStdString()),
                        webpier::locale_to_utf8(ca.ToStdString())
                    } 
                };

                CheckRendezvous(mediator, mediator, callback);
            }
            catch (const std::exception& ex)
            {
                callback(ex.what());
            }
        }
    }
}
