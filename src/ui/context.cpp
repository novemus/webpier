#include "../store/context.h"
#include "context.h"
#include "messagedialog.h"
#include "startupdialog.h"
#include <boost/property_tree/json_parser.hpp>
#include <wx/utils.h> 
#include <wx/stdpaths.h>
#include <wx/dataview.h>
#include <wx/regex.h>

namespace WebPier
{
    std::shared_ptr<webpier::context> GetContext()
    {
        static std::shared_ptr<webpier::context> s_context;

        if (s_context)
            return s_context;

        wxString home;
        if (!wxGetEnv("WEBPIER_HOME", &home))
            home = wxStandardPaths::Get().GetUserLocalDataDir();

        if (!wxFileName::Exists(home) && !wxFileName::Mkdir(home))
        {
            CMessageDialog dialog(nullptr, _("can't create home directory ") + home, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
            dialog.ShowModal();
            throw webpier::usage_error("no context");
        }

        auto context = webpier::open_context(home.ToStdString());

        webpier::config config;
        context->get_config(config);

        if (config.pier.empty())
        {
            CStartupDialog dialog(nullptr);
            if (dialog.ShowModal() == wxID_OK)
                config.pier = dialog.GetIdentity().ToStdString();

            if (config.pier.empty())
                throw webpier::usage_error("can't init context");

            context->set_config(config);
        }

        s_context = context;
        return context;
    }

    bool IsEqual(ServicePtr lhs, ServicePtr rhs)
    {
        return lhs && rhs 
            && lhs->Name == rhs->Name
            && lhs->Pier == rhs->Pier
            && lhs->Address == rhs->Address
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
                Name.ToStdString(),
                Pier.ToStdString(),
                Address.ToStdString(),
                Rendezvous.ToStdString(),
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
            m_origin = {};
        }

        void Revert() noexcept(true) override
        {
            Name = m_origin.name;
            Pier = m_origin.pier;
            Address = m_origin.address;
            Rendezvous = m_origin.rendezvous;
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
            return Name != m_origin.name
                || Pier != m_origin.pier
                || Address != m_origin.address
                || Rendezvous != m_origin.rendezvous
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
                Pier.ToStdString(),
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
                },
                Autostart
            };

            GetContext()->set_config(actual);

            m_origin = actual;
        }

        void Revert() noexcept(true) override
        {
            Pier = m_origin.pier;
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
            Autostart = m_origin.autostart;
        }
    };

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
            array.Add(item);
        return array;
    }

    bool IsUselessPier(const wxString& id) noexcept(false)
    {
        auto isUsedForRemote = [&]()
        {
            std::vector<webpier::service> list;
            GetContext()->get_import_services(list);
            auto iter = std::find_if(list.begin(), list.end(), [&id](const auto& item)
            {
                return item.pier == id;
            });
            return iter != list.end();
        };

        auto isUsedForLocal = [&]()
        {
            std::vector<webpier::service> list;
            GetContext()->get_export_services(list);
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
        GetContext()->add_pier(id.ToStdString(), cert.ToStdString());
    }

    void DelPier(const wxString& id) noexcept(false)
    {
        GetContext()->del_pier(id.ToStdString());
    }

    wxString GetCertificate(const wxString& id) noexcept(false)
    {
        return GetContext()->get_certificate(id.ToStdString());
    }

    wxString GetFingerprint(const wxString& id) noexcept(false)
    {
        return GetContext()->get_fingerprint(id.ToStdString());
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
            item.put("name", pair.second->Name.ToStdString());
            item.put("obscure", pair.second->Obscure);
            item.put("rendezvous", pair.second->Rendezvous.ToStdString());
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
            service->Name = item.second.get<std::string>("name");
            service->Pier = data.Pier;
            service->Obscure = item.second.get<bool>("obscure");
            service->Rendezvous = item.second.get<std::string>("rendezvous", "");
            data.Services[wxUIntPtr(service.get())] = service;
        }
    }
}
