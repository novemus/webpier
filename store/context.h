#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace webpier
{
    struct save_error : public std::runtime_error { save_error() : std::runtime_error("save error") {} };
    struct file_error : public std::runtime_error { file_error() : std::runtime_error("file error") {} };
    struct lock_error : public std::runtime_error { lock_error() : std::runtime_error("lock error") {} };

    enum log
    {
        fatal,
        error,
        warning,
        info,
        debug,
        trace
    };

    struct stun
    {
        std::string server;
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

    struct options
    {
        std::string owner;
        std::string pier;
        bool autostart;
        bool autotray;
        log logging;
        stun traverse;
        dht rendezvous;
        email emailer;
    };

    struct service
    {
        std::string id;
        std::string peer;
        std::string service;
        std::string gateway;
        dht rendezvous;
        bool autostart;
        bool obscure;
    };

    struct context
    {
        virtual ~context() {}

        virtual void renew() noexcept(false) = 0;
        virtual void flush(bool force = false) const noexcept(false) = 0;

        virtual void get_options(options& data) const noexcept(true) = 0;
        virtual void set_options(const options& data) noexcept(true) = 0;

        virtual void get_local_services(std::vector<service>& data) const noexcept(true) = 0;
        virtual void set_local_services(const std::vector<service>& data) noexcept(true) = 0;

        virtual void del_local_service(const service& data) noexcept(true) = 0;
        virtual void set_local_service(const service& data) noexcept(true) = 0;

        virtual void get_remote_services(std::vector<service>& data) const noexcept(true) = 0;
        virtual void set_remote_services(const std::vector<service>& data) noexcept(true) = 0;

        virtual void del_remote_service(const service& data) noexcept(true) = 0;
        virtual void set_remote_service(const service& data) noexcept(true) = 0;
    };

    std::shared_ptr<context> open_context(const std::string& config) noexcept(false);
}
