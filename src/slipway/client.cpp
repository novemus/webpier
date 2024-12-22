#include "client.h"
#include "channel.h"
#include <fstream>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/process/async_pipe.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace slipway
{
    constexpr const char* push_pipe_name = "client.pipe";
    constexpr const char* pull_pipe_name = "server.pipe";
    constexpr const char* lock_file_name = "client.lock";

    class client_impl : public client
    {
        boost::interprocess::file_lock m_locker;
        std::shared_ptr<channel> m_channel;

        template<class result = std::string>
        result submit(const message& data)
        {
            boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(m_locker);

            message back;

            m_channel->push(data);
            m_channel->pull(back);

            if (!back.ok())
                throw task_error(std::get<std::string>(back.payload));

            return std::get<result>(back.payload);
        }

        static boost::interprocess::file_lock open_lock_file(const std::string& file)
        {
            if (!std::filesystem::exists(file))
                std::ofstream(file).close();

            return boost::interprocess::file_lock(file.c_str());
        }

    public:

        client_impl(const std::string& home)
            : m_locker(open_lock_file(home + "/" + lock_file_name))
            , m_channel(create_channel(home + "/" + pull_pipe_name, home + "/" + push_pipe_name))
        {
        }

        void launch() noexcept(false) override
        {
            submit(message::make(message::launch));
        }

        void finish() noexcept(false) override
        {
            submit(message::make(message::finish));
        }

        void reboot() noexcept(false) override
        {
            submit(message::make(message::reboot));
        }

        void status(std::vector<slipway::wealth>& result) noexcept(false) override
        {
            result = submit<std::vector<slipway::wealth>>(message::make(message::status));
        }

        void review(std::vector<slipway::report>& result) noexcept(false) override
        {
            result = submit<std::vector<slipway::report>>(message::make(message::review));
        }

        void launch(const handle& service) noexcept(false) override
        {
            submit(message::make(message::launch, service));
        }

        void finish(const handle& service) noexcept(false) override
        {
            submit(message::make(message::finish, service));
        }

        void reboot(const handle& service) noexcept(false) override
        {
            submit(message::make(message::reboot, service));
        }

        void status(const handle& service, slipway::wealth& result) noexcept(false) override
        {
            result = submit<slipway::wealth>(message::make(message::status, service));
        }

        void review(const handle& service, slipway::report& result) noexcept(false) override
        {
            result = submit<slipway::report>(message::make(message::review, service));
        }
    };

    std::shared_ptr<client> open_client(const std::string& home) noexcept(false)
    {
        return std::make_shared<client_impl>(home);
    }
}
