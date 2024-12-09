#include "client.h"
#include "channel.h"
#include "../utils.h"
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/process/async_pipe.hpp>

namespace slipway
{
    constexpr const char* req_pipe_name = "slipway.0";
    constexpr const char* res_pipe_name = "slipway.1";
    constexpr const char* lock_file_name = "lock";

    class client_impl : public client
    {
        std::string m_locker;
        std::shared_ptr<channel> m_channel;

        template<class result = std::string>
        result transmit(const message& info)
        {
            webpier::locker lock(m_locker);

            message back;

            m_channel->push(info);
            m_channel->pull(back);

            if (!back.ok())
                throw task_error(std::get<std::string>(back.payload));

            return std::get<result>(back.payload);
        }

    public:

        client_impl(const std::string& context)
            : m_locker(context + "/" + lock_file_name)
            , m_channel(create_channel(context + "/" + res_pipe_name, context + "/" + req_pipe_name))
        {
        }

        void launch() noexcept(false) override
        {
            transmit(message::make(message::launch));
        }

        void finish() noexcept(false) override
        {
            transmit(message::make(message::finish));
        }

        void reboot() noexcept(false) override
        {
            transmit(message::make(message::reboot));
        }

        void status(std::vector<slipway::wealth>& result) noexcept(false) override
        {
            result = transmit<std::vector<slipway::wealth>>(message::make(message::status));
        }

        void review(std::vector<slipway::report>& result) noexcept(false) override
        {
            result = transmit<std::vector<slipway::report>>(message::make(message::review));
        }

        void launch(const handle& service) noexcept(false) override
        {
            transmit(message::make(message::launch, service));
        }

        void finish(const handle& service) noexcept(false) override
        {
            transmit(message::make(message::finish, service));
        }

        void reboot(const handle& service) noexcept(false) override
        {
            transmit(message::make(message::reboot, service));
        }

        void status(const handle& service, slipway::wealth& result) noexcept(false) override
        {
            result = transmit<slipway::wealth>(message::make(message::status, service));
        }

        void review(const handle& service, slipway::report& result) noexcept(false) override
        {
            result = transmit<slipway::report>(message::make(message::review, service));
        }
    };

    std::shared_ptr<client> open_client(const std::string& context) noexcept(false)
    {
        return std::make_shared<client_impl>(context);
    }
}
