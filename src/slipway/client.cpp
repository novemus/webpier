#include "slipway.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

namespace slipway
{
    namespace 
    {
        constexpr const char* jack_file_name = "slipway.jack";

        class client : public daemon
        {
            boost::asio::io_context m_io;
            boost::asio::local::stream_protocol::socket m_socket;

            void execute(const std::function<void(boost::asio::yield_context yield)>& function) noexcept(false)
            {
                m_io.restart();

                boost::asio::spawn(m_io, [&](boost::asio::yield_context yield)
                {
                    boost::asio::deadline_timer timer(m_io);

                    timer.expires_from_now(boost::posix_time::seconds(30));
                    timer.async_wait([&](const boost::system::error_code& error)
                    {
                        if(error)
                        {
                            if (error == boost::asio::error::operation_aborted)
                                return;

                            std::cerr << error.message() << std::endl;
                        }

                        try
                        {
                            m_socket.cancel();
                        }
                        catch (const std::exception &ex)
                        {
                            std::cerr << ex.what() << std::endl;
                        }
                    });

                    function(yield);

                    boost::system::error_code ec;
                    timer.cancel(ec);
                });

                m_io.run();
            }

            template<class result = std::string>
            result perform(const message& request)
            {
                message response;

                execute([&](boost::asio::yield_context yield)
                {
                    boost::asio::streambuf buffer;
                    push_message(buffer, request);

                    boost::system::error_code ec;
                    boost::asio::async_write(m_socket, buffer, yield[ec]);

                    if (ec)
                        throw pipe_error(ec.message());

                    boost::asio::async_read_until(m_socket, buffer, '\n', yield[ec]);

                    if (ec)
                        throw pipe_error(ec.message());

                    pull_message(buffer, response);
                });

                if (!response.ok())
                    throw task_error(std::get<std::string>(response.payload));

                return std::get<result>(response.payload);
            }

        public:

            client(const std::string& home)
                : m_socket(m_io)
            {
                execute([&](boost::asio::yield_context yield)
                {
                    boost::system::error_code ec;
                    m_socket.async_connect(home + "/" + jack_file_name, yield[ec]);

                    if (ec)
                        throw pipe_error(ec.message());
                });
            }

            void unplug() noexcept(false) override
            {
                perform(message::make(message::unplug));
            }

            void engage() noexcept(false) override
            {
                perform(message::make(message::engage));
            }

            void reboot() noexcept(false) override
            {
                perform(message::make(message::reboot));
            }

            void status(std::vector<slipway::health>& result) noexcept(false) override
            {
                perform<std::vector<slipway::health>>(message::make(message::status));
            }

            void review(std::vector<slipway::report>& result) noexcept(false) override
            {
                perform<std::vector<slipway::report>>(message::make(message::review));
            }

            void unplug(const handle& service) noexcept(false) override
            {
                perform(message::make(message::unplug, service));
            }

            void engage(const handle& service) noexcept(false) override
            {
                perform(message::make(message::engage, service));
            }

            void reboot(const handle& service) noexcept(false) override
            {
                perform(message::make(message::reboot, service));
            }

            void status(const handle& service, slipway::health& result) noexcept(false) override
            {
                perform<slipway::health>(message::make(message::status, service));
            }

            void review(const handle& service, slipway::report& result) noexcept(false) override
            {
                perform<slipway::report>(message::make(message::review, service));
            }
        };
    }

    std::shared_ptr<daemon> create_client(const std::string& home) noexcept(false)
    {
        return std::make_shared<client>(home);
    }
}
