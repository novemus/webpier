#pragma once

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/filename.h>
#include <wx/hashmap.h>
#include <wx/hashset.h>
#include <wx/sharedptr.h>

namespace WebPier
{
    struct Service
    {
        wxString Name;
        wxString Pier;
        wxString Address;
        wxString Rendezvous;
        bool Autostart;
        bool Obscure;

        virtual ~Service() {}
        virtual void Store() noexcept(false) = 0;
        virtual void Purge() noexcept(false) = 0;
        virtual void Revert() noexcept(true) = 0;
        virtual void AddPier(const wxString& pier) noexcept(true) = 0;
        virtual void DelPier(const wxString& pier) noexcept(true) = 0;
        virtual bool HasPier(const wxString& pier) const noexcept(true) = 0;
        virtual bool IsDirty() const noexcept(true) = 0;
        virtual bool IsExport() const noexcept(true) = 0;
        virtual bool IsImport() const noexcept(true) = 0;
    };

    using ServicePtr = wxSharedPtr<Service>;
    WX_DECLARE_HASH_MAP(wxUIntPtr, ServicePtr, wxIntegerHash, wxIntegerEqual, ServiceList);

    ServicePtr CreateExportService();
    ServicePtr CreateImportService();

    bool IsEqual(ServicePtr lhs, ServicePtr rhs);

    struct Config
    {
        wxString Pier;
        wxString StunServer;
        wxUint8 PunchHops;
        wxString DhtBootstrap;
        wxUint16 DhtPort;
        wxString SmtpServer;
        wxString ImapServer;
        wxString EmailLogin;
        wxString EmailPassword;
        wxString EmailX509Cert;
        wxString EmailX509Key;
        wxString EmailX509Ca;
        bool Autostart;

        virtual ~Config() {}
        virtual void Store() noexcept(false) = 0;
        virtual void Revert() noexcept(true) = 0;
    };

    using ConfigPtr = wxSharedPtr<Config>;

    struct Exchange
    {
        wxString Pier;
        wxString Certificate;
        ServiceList Services;
    };

    ConfigPtr GetConfig() noexcept(false);
    ServiceList GetExportServices() noexcept(false);
    ServiceList GetImportServices() noexcept(false);
    wxArrayString GetPiers() noexcept(false);
    bool IsUselessPier(const wxString& pier) noexcept(false);
    bool IsUnknownPier(const wxString& pier) noexcept(false);
    void AddPier(const wxString& pier, const wxString& cert) noexcept(false);
    void DelPier(const wxString& pier) noexcept(false);
    wxString GetCertificate(const wxString& pier) noexcept(false);
    wxString GetFingerprint(const wxString& pier) noexcept(false);
    void WriteExchangeFile(const wxString& file, const Exchange& data) noexcept(false);
    void ReadExchangeFile(const wxString& file, Exchange& data) noexcept(false);
}
