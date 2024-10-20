#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace webpier
{
    constexpr const char* default_stun_server = "stun.ekiga.net";
    constexpr const char* default_dht_bootstrap = "bootstrap.jami.net:4222";
    constexpr const char* default_gateway = "0.0.0.0:0";

    struct stale_error : public std::runtime_error { stale_error(const std::string& what) : std::runtime_error(what) {} };
    struct usage_error : public std::runtime_error { usage_error(const std::string& what) : std::runtime_error(what) {} };
    struct file_error : public std::runtime_error { file_error(const std::string& what) : std::runtime_error(what) {} };
    struct lock_error : public std::runtime_error { lock_error(const std::string& what) : std::runtime_error(what) {} };

    enum log
    {
        fatal,
        error,
        warning,
        info,
        debug,
        trace
    };

    std::ostream& operator<<(std::ostream& out, log level);
    std::istream& operator>>(std::istream& in, log& level);

    struct nat
    {
        std::string stun = default_stun_server;
        uint8_t hops = 7;
    };

    struct dht
    {
        std::string bootstrap = default_dht_bootstrap;
        uint32_t network = 0;
    };

    struct email
    {
        std::string smtp;
        std::string imap;
        std::string login;
        std::string password;
        std::string cert;
        std::string key;
        std::string ca;
    };

    struct config
    {
        std::string host;
        log report = log::info;
        bool daemon = false;
        bool tray = true;
        nat traverse;
        dht rendezvous;
        email emailer;
    };

    struct service
    {
        std::string id;
        std::string peer;
        std::string service;
        std::string gateway = default_gateway;
        dht rendezvous;
        bool autostart = false;
        bool obscure = true;
    };

    struct context
    {
        virtual ~context() {}

        virtual void get_peers(std::vector<std::string>& list) const noexcept(true) = 0;
        virtual void add_peer(const std::string& id, const std::string& cert) noexcept(false) = 0;
        virtual void del_peer(const std::string& id) noexcept(false) = 0;

        virtual void get_config(config& info) const noexcept(true) = 0;
        virtual void set_config(const config& info) noexcept(false) = 0;

        virtual void get_local_services(std::vector<service>& list) const noexcept(true) = 0;
        virtual void add_local_service(const service& info) noexcept(false) = 0;
        virtual void del_local_service(const std::string& id) noexcept(false) = 0;

        virtual void get_remote_services(std::vector<service>& list) const noexcept(true) = 0;
        virtual void add_remote_service(const service& info) noexcept(false) = 0;
        virtual void del_remote_service(const std::string& peer, const std::string& id) noexcept(false) = 0;

        virtual std::string get_certificate(const std::string& id) const noexcept(false) = 0;
        virtual std::string get_fingerprint(const std::string& id) const noexcept(false) = 0;
    };

    std::shared_ptr<context> open_context(const std::string& dir, const std::string& host) noexcept(false);
}
