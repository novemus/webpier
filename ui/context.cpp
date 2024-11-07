#include "../store/context.h"
#include "context.h"
#include "startupdialog.h"
#include "messagedialog.h"
#include <boost/property_tree/json_parser.hpp>
#include <wx/utils.h> 
#include <wx/stdpaths.h>
#include <wx/dataview.h>
#include <wx/regex.h>

namespace WebPier
{
    class Context
    {
        static std::shared_ptr<webpier::context>& getImpl()
        {
            static std::shared_ptr<webpier::context> s_context;
            return s_context;
        }

        static wxString getHome()
        {
            wxString home;
            if (!wxGetEnv("WEBPIER_HOME", &home))
                home = wxStandardPaths::Get().GetUserLocalDataDir();

            if (!wxFileName::Exists(home) && !wxFileName::Mkdir(home))
            {
                CMessageDialog dialog(nullptr, _("Can't create home directory ") + home, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
                dialog.ShowModal();
                throw webpier::usage_error("no context");
            }

            return home;
        }

        static wxString readRoute(const wxString& home)
        {
            wxString host;
            wxString route = home + "/webpier";

            if (wxFileName::Exists(route))
            {
                wxFile in(route, wxFile::read);
                if (!in.ReadAll(&host))
                {
                    CMessageDialog dialog(nullptr, _("Can't read context reference ") + route, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
                    dialog.ShowModal();
                    throw webpier::usage_error("no context");
                }
            }

            return host;
        }

        static void writeRoute(const wxString& home, const wxString& host)
        {
            wxString route = home + "/webpier";

            wxFile out(route, wxFile::write);
            if (!out.Write(host))
            {
                CMessageDialog dialog(nullptr, _("Can't write context reference ") + route, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
                dialog.ShowModal();
                throw webpier::usage_error("no context");
            }
        }

    public:

        static std::shared_ptr<webpier::context> Get()
        {
            auto& impl = getImpl();
            if (impl)
                return impl;

            wxString home = getHome();
            wxString host = readRoute(home);

            if (host.IsEmpty())
            {
                CStartupDialog dialog(nullptr);
                if (dialog.ShowModal() == wxID_OK)
                {
                    host = dialog.GetIdentity();
                    writeRoute(home, host);
                }
            }

            if (!host.IsEmpty())
                impl = webpier::open_context(home.ToStdString(), host.ToStdString());

            if (!impl)
                throw webpier::usage_error("no context");

            return impl;
        }

        static void Switch(const wxString& host)
        {
            auto& impl = getImpl();

            if (impl && impl->get_host() == host)
               return;

            impl.reset();

            writeRoute(getHome(), host);
        }

        static void Clear(const wxString& host)
        {
            auto& impl = getImpl();
            if (impl && impl->get_host() == host)
            {
                wxFileName::Rmdir(impl->get_home(), wxPATH_RMDIR_FULL|wxPATH_RMDIR_RECURSIVE);
                impl.reset();
                writeRoute(getHome(), "");
            }
        }
    };

    bool IsEqual(ServicePtr lhs, ServicePtr rhs)
    {
        return lhs && rhs 
            && lhs->Id == rhs->Id
            && lhs->Peer == rhs->Peer
            && lhs->Address == rhs->Address
            && lhs->Gateway == rhs->Gateway
            && lhs->DhtBootstrap == rhs->DhtBootstrap
            && lhs->DhtNetwork == rhs->DhtNetwork
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
                Id.ToStdString(),
                Peer.ToStdString(),
                Address.ToStdString(),
                Gateway.ToStdString(),
                { DhtBootstrap.ToStdString(), DhtNetwork },
                Autostart,
                Obscure
            };

            auto context = Context::Get();
            if (IsExport())
            {
                if (!m_origin.id.empty())
                    context->del_export_service(m_origin.id);
                context->add_export_service(actual);
            }
            else
            {
                if (!m_origin.id.empty() && !m_origin.peer.empty())
                    context->del_import_service(m_origin.peer, m_origin.id);
                context->add_import_service(actual);
            }
            m_origin = actual;
        }

        void Purge() noexcept(false) override
        {
            auto context = Context::Get();
            if (IsExport())
            {
                if (!m_origin.id.empty())
                    context->del_export_service(m_origin.id);
            }
            else
            {
                if (!m_origin.id.empty() && !m_origin.peer.empty())
                    context->del_import_service(m_origin.peer, m_origin.id);
            }
            m_origin = {};
        }

        void Revert() noexcept(true) override
        {
            Id = m_origin.id;
            Peer = m_origin.peer;
            Address = m_origin.address;
            Gateway = m_origin.gateway;
            DhtBootstrap = m_origin.rendezvous.bootstrap;
            DhtNetwork = m_origin.rendezvous.network;
            Autostart = m_origin.autostart;
            Obscure = m_origin.obscure;
        }

        void AddPeer(const wxString& peer) noexcept(true) override
        {
            if (IsExport())
            {
                auto arr = wxSplit(Peer, ' ');
                if (arr.Index(peer) == wxNOT_FOUND)
                {
                    arr.Add(peer);
                    Peer = wxJoin(arr, ' ');
                }
            }
            else
                Peer = peer;
        }

        void DelPeer(const wxString& peer) noexcept(true) override
        {
            if (IsExport())
            {
                auto arr = wxSplit(Peer, ' ');
                auto ind = arr.Index(peer);
                if (ind != wxNOT_FOUND)
                {
                    arr.RemoveAt(ind);
                    Peer = wxJoin(arr, ' ');
                }
            }
            else if (peer == Peer)
                Peer.Clear();
        }

        bool HasPeer(const wxString& peer) const noexcept(true) override
        {
            return IsExport() ? wxSplit(Peer, ' ').Index(peer) != wxNOT_FOUND : peer == Peer;
        }

        bool IsDirty() const noexcept(true) override
        {
            return Id != m_origin.id
                || Peer != m_origin.peer
                || Address != m_origin.address
                || Gateway != m_origin.gateway
                || DhtBootstrap != m_origin.rendezvous.bootstrap
                || DhtNetwork != m_origin.rendezvous.network
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
                Host.ToStdString(),
                { StunServer.ToStdString(), PunchHops },
                { DhtBootstrap.ToStdString(), DhtNetwork },
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

            Context::Switch(Host);
            Context::Get()->set_config(actual);
        }

        void Purge() noexcept(false) override
        {
            Context::Clear(m_origin.host);
        }

        void Revert() noexcept(true) override
        {
            Host = m_origin.host;
            StunServer = m_origin.traverse.stun;
            PunchHops = m_origin.traverse.hops;
            DhtBootstrap = m_origin.rendezvous.bootstrap;
            DhtNetwork = m_origin.rendezvous.network;
            SmtpServer = m_origin.emailer.smtp;
            ImapServer = m_origin.emailer.imap;
            EmailLogin = m_origin.emailer.login;
            EmailPassword = m_origin.emailer.password;
            EmailX509Cert = m_origin.emailer.cert;
            EmailX509Key = m_origin.emailer.key;
            EmailX509Ca = m_origin.emailer.ca;
        }
    };

    ConfigPtr GetConfig() noexcept(false)
    {
        webpier::config config;
        Context::Get()->get_config(config);
        return ConfigPtr(new ConfigImpl(config));
    }

    ServiceList GetExportServices() noexcept(false)
    {
        ServiceList collection;
        std::vector<webpier::service> list;
        Context::Get()->get_export_services(list);
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
        Context::Get()->get_import_services(list);
        for (const auto& item : list)
        {
            ServicePtr ptr(new ImportService(item));
            collection[wxUIntPtr(ptr.get())] = ptr;
        }

        return collection;
    }

    wxArrayString GetPeers() noexcept(false)
    {
        wxArrayString array;
        std::vector<std::string> list;
        Context::Get()->get_peers(list);
        for (const auto& item : list)
            array.Add(item);
        return array;
    }

    bool IsUselessPeer(const wxString& id) noexcept(false)
    {
        auto isUsedForRemote = [&]()
        {
            std::vector<webpier::service> list;
            Context::Get()->get_import_services(list);
            auto iter = std::find_if(list.begin(), list.end(), [&id](const auto& item)
            {
                return item.peer == id;
            });
            return iter != list.end();
        };

        auto isUsedForLocal = [&]()
        {
            std::vector<webpier::service> list;
            Context::Get()->get_export_services(list);
            auto iter = std::find_if(list.begin(), list.end(), [peer = id.ToStdString()](const auto& item)
            {
                return item.peer.find(peer) != std::string::npos;
            });
            return iter != list.end();
        };

        return !isUsedForRemote() && !isUsedForLocal();
    }

    bool IsUnknownPeer(const wxString& id) noexcept(false)
    {
        return !Context::Get()->has_peer(id.ToStdString());
    }

    void AddPeer(const wxString& id, const wxString& cert) noexcept(false)
    {
        Context::Get()->add_peer(id.ToStdString(), cert.ToStdString());
    }

    void DelPeer(const wxString& id) noexcept(false)
    {
        Context::Get()->del_peer(id.ToStdString());
    }

    wxString GetCertificate(const wxString& id) noexcept(false)
    {
        return Context::Get()->get_certificate(id.ToStdString());
    }

    wxString GetFingerprint(const wxString& id) noexcept(false)
    {
        return Context::Get()->get_fingerprint(id.ToStdString());
    }

    void WriteExchangeFile(const wxString& file, const Exchange& data) noexcept(false)
    {
        boost::property_tree::ptree doc;
        doc.put("pier", data.Pier.ToStdString());
        doc.put("certificate", data.Certificate.ToStdString());

        boost::property_tree::ptree array;
        for (const auto& pair : data.Services)
        {
            boost::property_tree::ptree item;
            item.put("id", pair.second->Id.ToStdString());
            item.put("obscure", pair.second->Obscure);
            item.put("rendezvous.dht.bootstrap", pair.second->DhtBootstrap.ToStdString());
            item.put("rendezvous.dht.network", pair.second->DhtNetwork);
            array.push_back(std::make_pair("", item));
        }
        doc.put_child("services", array);

        boost::property_tree::write_json(file.ToStdString(), doc);
    }

    void ReadExchangeFile(const wxString& file, Exchange& data) noexcept(false)
    {
        boost::property_tree::ptree doc;
        boost::property_tree::read_json(file.ToStdString(), doc);

        data.Pier = doc.get<std::string>("pier", "");
        data.Certificate = doc.get<std::string>("certificate", "");

        boost::property_tree::ptree array;
        for (auto& item : doc.get_child("services", array))
        {
            ServicePtr service(new ImportService());
            service->Id = item.second.get<std::string>("id");
            service->Peer = data.Pier;
            service->Obscure = item.second.get<bool>("obscure");
            service->DhtBootstrap = item.second.get<std::string>("rendezvous.dht.bootstrap", "");
            service->DhtNetwork = item.second.get<uint32_t>("rendezvous.dht.network", 0);
            data.Services[wxUIntPtr(service.get())] = service;
        }
    }
}
