#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace webpier
{
    struct stale_error : public std::runtime_error { stale_error(const std::string& what) : std::runtime_error(what) {} };
    struct file_error : public std::runtime_error { file_error(const std::string& what) : std::runtime_error(what) {} };
    struct lock_error : public std::runtime_error { lock_error(const std::string& what) : std::runtime_error(what) {} };
    struct unique_error : public std::runtime_error { unique_error(const std::string& what) : std::runtime_error(what) {} };

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
        std::string stun;
        uint8_t hops;
    };

    struct dht
    {
        std::string bootstrap;
        uint32_t network;
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

    struct basic
    {
        std::string host;
        log report;
        bool daemon;
        bool tray;
        nat traverse;
        dht rendezvous;
        email emailer;
    };

    struct service
    {
        std::string id;
        std::string peer;
        std::string mapping;
        std::string gateway;
        dht rendezvous;
        bool autostart;
        bool obscure;
    };

    struct context
    {
        virtual ~context() {}

        virtual void reload() noexcept(false) = 0;

        virtual void get_basic(basic& out) const noexcept(true) = 0;
        virtual void set_basic(const basic& info) noexcept(false) = 0;

        virtual void get_local_services(std::vector<service>& out) const noexcept(true) = 0;
        virtual void add_local_service(const service& info) noexcept(false) = 0;
        virtual void del_local_service(const std::string& id) noexcept(false) = 0;

        virtual void get_remote_services(std::vector<service>& out) const noexcept(true) = 0;
        virtual void add_remote_service(const service& info) noexcept(false) = 0;
        virtual void del_remote_service(const std::string& peer, const std::string& id) noexcept(false) = 0;

        virtual void get_peers(std::vector<std::string>& out) const noexcept(true) = 0;
        virtual void add_peer(const std::string& peer, const std::string& cert) noexcept(false) = 0;
        virtual void del_peer(const std::string& peer) noexcept(false) = 0;

        virtual std::string get_cert_hash(const std::string& identity) const noexcept(false) = 0;
    };

    std::shared_ptr<context> open_context(const std::string& dir) noexcept(false);
}
