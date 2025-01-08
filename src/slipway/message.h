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
        std::string pier;
        std::string service;

        bool operator<(const handle& other) const { return pier < other.pier || service < other.service; }
        bool operator==(const handle& other) const { return pier == other.pier && service == other.service; }
    };

    struct health : public handle
    {
        enum status
        {
            asleep,
            failed,
            active
        };

        status state;

        bool operator<(const health& other) const { return handle::operator<(other) || state < other.state; }
        bool operator==(const health& other) const { return handle::operator==(other) && state == other.state; }
    };

    struct report : public health
    {
        struct spawn
        {
            std::string pier;
            int pid;

            bool operator<(const spawn& other) const { return pier < other.pier || pid < other.pid; }
            bool operator==(const spawn& other) const { return pier == other.pier && pid == other.pid; }
        };

        std::vector<spawn> asset;

        bool operator<(const report& other) const { return health::operator<(other) || asset < other.asset; }
        bool operator==(const report& other) const { return health::operator==(other) && asset == other.asset; }
    };

    struct message
    {
        enum command
        {
            naught,
            unplug,
            engage,
            reboot,
            status,
            review
        };

        using content = std::variant<std::string, // error
                                     slipway::handle,
                                     slipway::health,
                                     slipway::report,
                                     std::vector<slipway::health>,
                                     std::vector<slipway::report>>;

        command action = command::naught;
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
            return action != command::naught && (payload.index() != 0 || std::get<std::string>(payload).empty());
        }
    };

    void push_message(boost::asio::streambuf& buffer, const message& message) noexcept(true);
    void pull_message(boost::asio::streambuf& buffer, message& message) noexcept(false);
}
