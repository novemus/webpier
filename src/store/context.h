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
    constexpr const uint16_t default_dht_port = 0;

    struct stale_error : public std::runtime_error { stale_error(const std::string& what) : std::runtime_error(what) {} };
    struct usage_error : public std::runtime_error { usage_error(const std::string& what) : std::runtime_error(what) {} };
    struct file_error : public std::runtime_error { file_error(const std::string& what) : std::runtime_error(what) {} };

    struct journal
    {
        enum severity
        {
            none,
            fatal,
            error,
            warning,
            info,
            debug,
            trace
        };

        std::string folder;
        severity level = debug;
    };

    struct puncher
    {
        std::string stun = default_stun_server;
        uint8_t hops = 7;
    };

    struct dhtnode
    {
        std::string bootstrap = default_dht_bootstrap;
        uint16_t port = default_dht_port;
        uint32_t network = 0;
    };

    struct emailer
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
        std::string pier;
        std::string repo;
        journal log;
        puncher nat;
        dhtnode dht;
        emailer email;
    };

    struct service
    {
        bool local;
        std::string name;
        std::string pier;
        std::string address;
        std::string gateway = default_gateway;
        std::string rendezvous = default_dht_bootstrap;
        bool autostart = false;
        bool obscure = true;
    };

    struct context
    {
        virtual ~context() {}

        virtual std::string pier() const noexcept(true) = 0;
        virtual std::string home() const noexcept(true) = 0;

        virtual void get_config(config& info) const noexcept(true) = 0;
        virtual void set_config(const config& info) noexcept(false) = 0;

        virtual void get_piers(std::vector<std::string>& list) const noexcept(true) = 0;
        virtual void add_pier(const std::string& pier, const std::string& cert) noexcept(false) = 0;
        virtual void del_pier(const std::string& pier) noexcept(false) = 0;

        virtual void get_export_services(std::vector<service>& list) const noexcept(true) = 0;
        virtual void get_import_services(std::vector<service>& list) const noexcept(true) = 0;

        virtual void add_export_service(const service& info) noexcept(false) = 0;
        virtual void del_export_service(const std::string& name) noexcept(false) = 0;

        virtual void add_import_service(const service& info) noexcept(false) = 0;
        virtual void del_import_service(const std::string& pier, const std::string& name) noexcept(false) = 0;

        virtual std::string get_certificate(const std::string& pier) const noexcept(false) = 0;
        virtual std::string get_fingerprint(const std::string& pier) const noexcept(false) = 0;
    };

    std::shared_ptr<context> open_context(const std::string& home) noexcept(false);
}
