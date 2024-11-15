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
        wxString Peer;
        wxString Address;
        wxString Gateway;
        wxString DhtBootstrap;
        wxUint32 DhtNetwork;
        bool Autostart;
        bool Obscure;

        virtual ~Service() {}
        virtual void Store() noexcept(false) = 0;
        virtual void Purge() noexcept(false) = 0;
        virtual void Revert() noexcept(true) = 0;
        virtual void AddPeer(const wxString& peer) noexcept(true) = 0;
        virtual void DelPeer(const wxString& peer) noexcept(true) = 0;
        virtual bool HasPeer(const wxString& peer) const noexcept(true) = 0;
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
        wxString Host;
        wxString StunServer;
        wxUint8 PunchHops;
        wxString DhtBootstrap;
        wxUint32 DhtNetwork;
        wxString SmtpServer;
        wxString ImapServer;
        wxString EmailLogin;
        wxString EmailPassword;
        wxString EmailX509Cert;
        wxString EmailX509Key;
        wxString EmailX509Ca;
        bool Autostart;

        virtual ~Config() {}
        virtual void Store(bool tidy) noexcept(false) = 0;
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
    wxArrayString GetPeers() noexcept(false);
    bool IsUselessPeer(const wxString& peer) noexcept(false);
    bool IsUnknownPeer(const wxString& peer) noexcept(false);
    void AddPeer(const wxString& peer, const wxString& cert) noexcept(false);
    void DelPeer(const wxString& peer) noexcept(false);
    wxString GetCertificate(const wxString& peer) noexcept(false);
    wxString GetFingerprint(const wxString& peer) noexcept(false);
    void WriteExchangeFile(const wxString& file, const Exchange& data) noexcept(false);
    void ReadExchangeFile(const wxString& file, Exchange& data) noexcept(false);
}
