#pragma once

#include <wx/string.h>
#include <wx/vector.h>
#include <wx/arrstr.h>
#include <wx/variant.h>
#include <wx/filename.h>
#include <wx/hashset.h>
#include <wx/sharedptr.h>
#include "../store/context.h"

namespace WebPier
{
    class Service
    {
        bool m_local;
        webpier::service m_origin;
        webpier::service m_midway;

    public:

        Service(bool local, const webpier::service& service = {}) : m_local(local), m_origin(service), m_midway(service) {}
        void Store() noexcept(false);
        void Purge() noexcept(false);
        void Revert() noexcept(true) { m_midway = m_origin; }
        void UseDhtRendezvous() noexcept(true);
        void UseEmailRendezvous() noexcept(true);
        wxVector<wxVariant> ToListView() const noexcept(true);
        wxString GetId() const noexcept(true) { return m_midway.id; }
        wxString GetGateway() const noexcept(true) { return m_midway.gateway; }
        wxString GetPeer() noexcept(true) { return m_midway.peer; }
        wxArrayString GetPees() noexcept(true);
        wxString GetService() const noexcept(true) { return m_midway.service; }
        wxString GetDhtBootstrap() const noexcept(true) { return m_midway.rendezvous.bootstrap; }
        wxUint32 GetDhtNetwork() const noexcept(true) { return m_midway.rendezvous.network; }
        bool IsLocal() const noexcept(true) { return m_local; }
        bool IsRemote() const noexcept(true) { return !m_local; }
        bool IsAutostart() const noexcept(true) { return m_midway.autostart; }
        bool IsObscure() const noexcept(true) { return m_midway.obscure; }
        bool IsEmailRendezvous() const noexcept(true) { return m_origin.rendezvous.bootstrap.empty(); }
        bool IsDhtRendezvous() const noexcept(true) { return !m_origin.rendezvous.bootstrap.empty(); }
        void SetId(const wxString& id) noexcept(true) { m_midway.id = id; }
        void SetGateway(const wxString& gateway) noexcept(true) { m_midway.gateway = gateway; }
        void SetPeer(const wxString& peer) noexcept(true) { m_midway.peer = peer; }
        void SetPeers(const wxArrayString& peers) noexcept(true);
        void SetService(const wxString& service) noexcept(true) { m_midway.service = service; }
        void SetDhtBootstrap(const wxString& bootstrap) noexcept(true) { m_midway.rendezvous.bootstrap = bootstrap; }
        void SetDhtNetwork(wxUint32 network) noexcept(true) { m_midway.rendezvous.network = network; }
        void SetAutostart(bool state) noexcept(true) { m_midway.autostart = state; }
        void SetObscure(bool state) noexcept(true) { m_midway.obscure = state; }
    };

    class Config
    {
        webpier::config m_origin;
        webpier::config m_midway;

    public:

        Config(const webpier::config& config = {}) : m_origin(config), m_midway(config) { }
        void Store(bool tidy = false) noexcept(false);
        void Revert() noexcept(true) { m_midway = m_origin; }
        wxString GetHost() const noexcept(true) { return m_midway.host; }
        wxString GetOwner() const noexcept(true);
        wxString GetPier() const noexcept(true);
        bool IsDaemon() const noexcept(true) { return m_midway.daemon; }
        bool IsTray() const noexcept(true) { return m_midway.tray; }
        wxString GetStunServer() const noexcept(true) { return m_midway.traverse.stun; }
        wxUint8 GetPunchHops() const noexcept(true) { return m_midway.traverse.hops; }
        wxString GetDhtBootstrap() const noexcept(true) { return m_midway.rendezvous.bootstrap; }
        wxUint32 GetDhtNetwork() const noexcept(true) { return m_midway.rendezvous.network; }
        wxString GetSmtpServer() const noexcept(true) { return m_midway.emailer.smtp; }
        wxString GetImapServer() const noexcept(true) { return m_midway.emailer.imap; }
        wxString GetEmailLogin() const noexcept(true) { return m_midway.emailer.login; }
        wxString GetEmailPassword() const noexcept(true) { return m_midway.emailer.password; }
        wxString GetEmailX509Cert() const noexcept(true) { return m_midway.emailer.cert; }
        wxString GetEmailX509Key() const noexcept(true) { return m_midway.emailer.key; }
        wxString GetEmailX509Ca() const noexcept(true) { return m_midway.emailer.ca; }
        void SetHost(const wxString& host) noexcept(true) { m_midway.host = host; }
        void SetHost(const wxString& owner, const wxString& pier) noexcept(true) { m_midway.host = owner + "/" + pier; }
        void SetOwner(const wxString& owner) noexcept(true) { m_midway.host = owner + "/" + GetPier(); }
        void SetPier(const wxString& pier) noexcept(true) { m_midway.host = GetOwner() + "/" + pier; }
        void SetDaemon(bool state) noexcept(true) { m_midway.daemon = state; }
        void SetTray(bool state) noexcept(true) { m_midway.tray = state; }
        void SetStunServer(const wxString& stun) noexcept(true) { m_midway.traverse.stun = stun; }
        void SetPunchHops(wxUint8 hops) noexcept(true) { m_midway.traverse.hops = hops; }
        void SetDhtBootstrap(const wxString& bootstrap) noexcept(true) { m_midway.rendezvous.bootstrap = bootstrap; }
        void SetDhtNetwork(wxUint32 network) noexcept(true) { m_midway.rendezvous.network = network; }
        void SetSmtpServer(const wxString& smtp) noexcept(true) { m_midway.emailer.smtp = smtp; }
        void SetImapServer(const wxString& imap) noexcept(true) { m_midway.emailer.imap = imap; }
        void SetEmailLogin(const wxString& login) noexcept(true) { m_midway.emailer.login = login; }
        void SetEmailPassword(const wxString& password) noexcept(true) { m_midway.emailer.password = password; }
        void SetEmailX509Cert(const wxString& cert) noexcept(true) { m_midway.emailer.cert = cert; }
        void SetEmailX509Key(const wxString& key) noexcept(true) { m_midway.emailer.key = key; }
        void SetEmailX509Ca(const wxString& ca) noexcept(true) { m_midway.emailer.ca = ca; }
    };

    wxString GetHost() noexcept(false);
    Config GetConfig() noexcept(false);
    wxVector<Service> GetLocalServices() noexcept(false);
    wxVector<Service> GetRemoteServices() noexcept(false);
    wxArrayString GetPeers() noexcept(false);
    bool IsUnusedPeer(const wxString& id) noexcept(false);
    void AddPeer(const wxString& id, const wxString& cert) noexcept(false);
    void DelPeer(const wxString& id) noexcept(false);
    wxString GetCertificate(const wxString& id) noexcept(false);
    wxString GetFingerprint(const wxString& id) noexcept(false);
}
