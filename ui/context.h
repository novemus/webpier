#pragma once

#include <wx/string.h>
#include <wx/vector.h>
#include <wx/arrstr.h>
#include <wx/variant.h>
#include <wx/filename.h>
#include <wx/hashmap.h>
#include <wx/sharedptr.h>
#include "../store/context.h"

namespace WebPier
{
    class Service
    {
        bool m_local;
        webpier::service m_origin;
        webpier::service m_actual;

    public:

        Service(bool local = false, const webpier::service& service = {}) : m_local(local), m_origin(service), m_actual(service) {}
        void Store() noexcept(false);
        void Purge() noexcept(false);
        void Revert() noexcept(true) { m_actual = m_origin; }
        void UseDhtRendezvous() noexcept(true);
        void UseEmailRendezvous() noexcept(true);
        wxVector<wxVariant> ToListView() const noexcept(true);
        wxString GetId() const noexcept(true) { return m_actual.id; }
        wxString GetGateway() const noexcept(true) { return m_actual.gateway; }
        wxString GetPeer() const noexcept(true) { return m_actual.peer; }
        wxArrayString GetPees() const noexcept(true);
        wxString GetService() const noexcept(true) { return m_actual.service; }
        wxString GetDhtBootstrap() const noexcept(true) { return m_actual.rendezvous.bootstrap; }
        wxUint32 GetDhtNetwork() const noexcept(true) { return m_actual.rendezvous.network; }
        bool IsPeerPresent(const wxString& peer) const noexcept(true);
        bool IsLocal() const noexcept(true) { return m_local; }
        bool IsRemote() const noexcept(true) { return !m_local; }
        bool IsAutostart() const noexcept(true) { return m_actual.autostart; }
        bool IsObscure() const noexcept(true) { return m_actual.obscure; }
        bool IsEmailRendezvous() const noexcept(true) { return m_origin.rendezvous.bootstrap.empty(); }
        bool IsDhtRendezvous() const noexcept(true) { return !m_origin.rendezvous.bootstrap.empty(); }
        void SetId(const wxString& id) noexcept(true) { m_actual.id = id; }
        void SetGateway(const wxString& gateway) noexcept(true) { m_actual.gateway = gateway; }
        void SetPeer(const wxString& peer) noexcept(true) { m_actual.peer = peer; }
        void AddPeer(const wxString& peer) noexcept(true);
        void DelPeer(const wxString& peer) noexcept(true);
        void SetPeers(const wxArrayString& peers) noexcept(true);
        void SetService(const wxString& service) noexcept(true) { m_actual.service = service; }
        void SetDhtBootstrap(const wxString& bootstrap) noexcept(true) { m_actual.rendezvous.bootstrap = bootstrap; }
        void SetDhtNetwork(wxUint32 network) noexcept(true) { m_actual.rendezvous.network = network; }
        void SetAutostart(bool state) noexcept(true) { m_actual.autostart = state; }
        void SetObscure(bool state) noexcept(true) { m_actual.obscure = state; }
        bool IsEqual(const Service& other) const noexcept(true);
        bool IsDirty() const noexcept(true);
    };

    using ServicePtr = wxSharedPtr<Service>;
    WX_DECLARE_HASH_MAP(wxUIntPtr, ServicePtr, wxIntegerHash, wxIntegerEqual, ServiceList);

    class Config
    {
        webpier::config m_origin;
        webpier::config m_actual;

    public:

        Config(const webpier::config& config = {}) : m_origin(config), m_actual(config) { }
        void Store(bool tidy = false) noexcept(false);
        void Revert() noexcept(true) { m_actual = m_origin; }
        wxString GetHost() const noexcept(true) { return m_actual.host; }
        wxString GetOwner() const noexcept(true);
        wxString GetPier() const noexcept(true);
        bool IsDaemon() const noexcept(true) { return m_actual.daemon; }
        wxString GetStunServer() const noexcept(true) { return m_actual.traverse.stun; }
        wxUint8 GetPunchHops() const noexcept(true) { return m_actual.traverse.hops; }
        wxString GetDhtBootstrap() const noexcept(true) { return m_actual.rendezvous.bootstrap; }
        wxUint32 GetDhtNetwork() const noexcept(true) { return m_actual.rendezvous.network; }
        wxString GetSmtpServer() const noexcept(true) { return m_actual.emailer.smtp; }
        wxString GetImapServer() const noexcept(true) { return m_actual.emailer.imap; }
        wxString GetEmailLogin() const noexcept(true) { return m_actual.emailer.login; }
        wxString GetEmailPassword() const noexcept(true) { return m_actual.emailer.password; }
        wxString GetEmailX509Cert() const noexcept(true) { return m_actual.emailer.cert; }
        wxString GetEmailX509Key() const noexcept(true) { return m_actual.emailer.key; }
        wxString GetEmailX509Ca() const noexcept(true) { return m_actual.emailer.ca; }
        void SetHost(const wxString& host) noexcept(true) { m_actual.host = host; }
        void SetHost(const wxString& owner, const wxString& pier) noexcept(true) { m_actual.host = owner + "/" + pier; }
        void SetOwner(const wxString& owner) noexcept(true) { m_actual.host = owner + "/" + GetPier(); }
        void SetPier(const wxString& pier) noexcept(true) { m_actual.host = GetOwner() + "/" + pier; }
        void SetDaemon(bool state) noexcept(true) { m_actual.daemon = state; }
        void SetStunServer(const wxString& stun) noexcept(true) { m_actual.traverse.stun = stun; }
        void SetPunchHops(wxUint8 hops) noexcept(true) { m_actual.traverse.hops = hops; }
        void SetDhtBootstrap(const wxString& bootstrap) noexcept(true) { m_actual.rendezvous.bootstrap = bootstrap; }
        void SetDhtNetwork(wxUint32 network) noexcept(true) { m_actual.rendezvous.network = network; }
        void SetSmtpServer(const wxString& smtp) noexcept(true) { m_actual.emailer.smtp = smtp; }
        void SetImapServer(const wxString& imap) noexcept(true) { m_actual.emailer.imap = imap; }
        void SetEmailLogin(const wxString& login) noexcept(true) { m_actual.emailer.login = login; }
        void SetEmailPassword(const wxString& password) noexcept(true) { m_actual.emailer.password = password; }
        void SetEmailX509Cert(const wxString& cert) noexcept(true) { m_actual.emailer.cert = cert; }
        void SetEmailX509Key(const wxString& key) noexcept(true) { m_actual.emailer.key = key; }
        void SetEmailX509Ca(const wxString& ca) noexcept(true) { m_actual.emailer.ca = ca; }
    };

    struct Exchange
    {
        wxString pier;
        wxString certificate;
        ServiceList services;
    };

    wxString GetHost() noexcept(false);
    Config GetConfig() noexcept(false);
    ServiceList GetLocalServices() noexcept(false);
    ServiceList GetRemoteServices() noexcept(false);
    ServicePtr GetLocalService(const wxString& id) noexcept(false);
    ServicePtr GetRemoteService(const wxString& peer, const wxString& id) noexcept(false);
    wxArrayString GetPeers() noexcept(false);
    bool IsUselessPeer(const wxString& id) noexcept(false);
    bool IsPeerExist(const wxString& id) noexcept(false);
    void AddPeer(const wxString& id, const wxString& cert) noexcept(false);
    void DelPeer(const wxString& id) noexcept(false);
    wxString GetCertificate(const wxString& id) noexcept(false);
    wxString GetFingerprint(const wxString& id) noexcept(false);
    void WriteExchangeFile(const wxString& file, const Exchange& data) noexcept(false);
    void ReadExchangeFile(const wxString& file, Exchange& data) noexcept(false);
}
