#pragma once

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/filename.h>
#include <wx/hashmap.h>
#include <wx/hashset.h>
#include <wx/sharedptr.h>

namespace WebPier
{
    bool Init();

    namespace Context
    {
        struct Service
        {
            bool Local;
            wxString Name;
            wxString Pier;
            wxString Address;
            wxString Gateway;
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
        };

        using ServicePtr = wxSharedPtr<Service>;
        WX_DECLARE_HASH_MAP(wxUIntPtr, ServicePtr, wxIntegerHash, wxIntegerEqual, ServiceList);

        ServicePtr CreateExportService();
        ServicePtr CreateImportService();

        bool IsEqual(ServicePtr lhs, ServicePtr rhs);

        struct Config
        {
            enum Logging
            {
                none,
                fatal,
                error,
                warning,
                info,
                debug,
                trace
            };

            wxString Pier;
            wxString Repo;
            wxString LogFolder;
            Logging LogLevel;
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

            virtual ~Config() {}
            virtual void Store() noexcept(false) = 0;
            virtual void Revert() noexcept(true) = 0;
        };

        using ConfigPtr = wxSharedPtr<Config>;

        struct Offer
        {
            wxString Pier;
            wxString Certificate;
            ServiceList Services;
        };

        wxString Pier() noexcept(false);
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
        void WriteOffer(const wxString& file, const Offer& offer) noexcept(false);
        void ReadOffer(const wxString& file, Offer& offer) noexcept(false);
    }

    namespace Backend
    {
        struct Handle
        {
            wxString Pier;
            wxString Service;
        };

        struct Health : public Handle
        {
            enum Status
            {
                Asleep,
                Broken,
                Lonely,
                Burden
            };

            Status State;
            wxString Message;
        };

        struct Report : public Health
        {
            struct Tunnel
            {
                std::string Pier;
                wxUint32 Pid;
            };

            wxVector<Tunnel> Tunnels;
        };

        void Unplug() noexcept(false);
        void Engage() noexcept(false);
        void Adjust() noexcept(false);
        wxVector<Health> Status() noexcept(false);
        wxVector<Report> Review() noexcept(false);
        void Unplug(const Handle& handle) noexcept(false);
        void Engage(const Handle& handle) noexcept(false);
        void Adjust(const Handle& handle) noexcept(false);
        Health Status(const Handle& handle) noexcept(false);
        Report Review(const Handle& handle) noexcept(false);
        void AssignAutostart() noexcept(false);
        void RevokeAutostart() noexcept(false);
        bool VerifyAutostart() noexcept(false);
        bool CouldAutostart() noexcept(false);
    }

    namespace Utils
    {
        struct NatState
        {
            enum Binding
            {
                Independent = 0,
                PortDependent = 1,
                AddressDependent = 2,
                AddressAndPortDependent = 3
            };

            wxString Error;
            bool Nat = false;
            bool Hairpin = false;
            bool RandomPort = false;
            bool VariableAddress = false;
            Binding Mapping;
            Binding Filtering;
            wxString InnerEndpoint;
            wxString OuterEndpoint;
        };

        void ExploreNat(const wxString& bind, const wxString& stun, const std::function<void(const NatState&)>& callback) noexcept(true);
        void CheckDhtRendezvous(const wxString& bootstrap, wxUint32 network, wxUint16 port, const std::function<void(const wxString&)>& callback) noexcept(true);
        void CheckEmailRendezvous(const wxString& smtp, const wxString& imap, const wxString& login, const wxString& password, const wxString& cert, const wxString& key, const wxString& ca, const std::function<void(const wxString&)>& callback) noexcept(true);
    }
}
