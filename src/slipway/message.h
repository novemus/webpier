#pragma once

#include <vector>
#include <variant>
#include <stdexcept>
#include <boost/asio.hpp>

namespace slipway
{
    struct task_error : public std::runtime_error { task_error(const std::string& what) : std::runtime_error(what) {} };
    struct pipe_error : public std::runtime_error { pipe_error(const std::string& what) : std::runtime_error(what) {} };

    struct handle
    {
        std::string node;
        std::string service;
    };

    struct outline : public handle
    {
        enum status
        {
            asleep,
            failed,
            active
        };

        status state;
    };

    struct snapshot : public outline
    {
        struct linkage
        {
            std::string peer;
            std::string logfile;
            int pid;
        };

        std::string logfile;
        std::vector<linkage> context;
    };

    struct message
    {
        enum command
        {
            reboot,
            launch,
            finish,
            status,
            report
        };

        using content = std::variant<std::string,
                                     slipway::handle,
                                     slipway::outline,
                                     slipway::snapshot,
                                     std::vector<slipway::outline>,
                                     std::vector<slipway::snapshot>>;

        command action;
        content payload;

        static message make(command action) noexcept(true)
        {
            return { action };
        }

        static message make(command action, content payload) noexcept(true)
        {
            return { action, std::move(payload) };
        }

        bool ok() const noexcept(true)
        {
            return payload.index() != 0 || std::get<0>(payload).empty();
        }
    };

    void put_message(boost::asio::streambuf& buffer, const message& message) noexcept(true);
    void get_message(boost::asio::streambuf& buffer, message& message) noexcept(false);
}
