#include <iostream>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <backend/client.h>

namespace slipway
{
    namespace 
    {
        constexpr const char* jack_file_name = "slipway.jack";

        class client : public backend
        {
            boost::asio::io_context m_io;
            boost::asio::local::stream_protocol::socket m_socket;

            void execute(const std::function<void(boost::asio::yield_context yield)>& function) noexcept(false)
            {
                m_io.restart();

                boost::asio::spawn(m_io, [&](boost::asio::yield_context yield)
                {
                    static constexpr const int REQUEST_TIMEOUT = 10;

                    boost::asio::deadline_timer timer(m_io);

                    timer.expires_from_now(boost::posix_time::seconds(REQUEST_TIMEOUT));
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
                            m_socket.close();
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
                        throw pipe_error("Can't write to " + m_socket.remote_endpoint().path() + " due the error \'" + ec.message() + "\'");

                    boost::asio::async_read_until(m_socket, buffer, '\n', yield[ec]);

                    if (ec)
                        throw pipe_error("Can't read from " + m_socket.remote_endpoint().path() + " due the error \'" + ec.message() + "\'");

                    pull_message(buffer, response);
                });

                if (!response.ok())
                    throw task_error("The server reported the error \'" + std::get<std::string>(response.payload) + "\'");

                return std::get<result>(response.payload);
            }

        public:

            client(const std::filesystem::path& home)
                : m_socket(m_io)
            {
                execute([&](boost::asio::yield_context yield)
                {
                    static constexpr const size_t MAX_ATTEMPTS = 5;

                    auto socket = home / jack_file_name;

                    boost::system::error_code ec;
                    for(size_t i = 0; i < MAX_ATTEMPTS; ++i)
                    {
#ifdef WIN32
                        if (ec.value() == WSAECONNREFUSED)
#else
                        if (ec.value() == boost::system::errc::no_such_file_or_directory || ec.value() == boost::system::errc::connection_refused)
#endif
                        {
                            boost::asio::deadline_timer timer(m_io);
                            timer.expires_from_now(boost::posix_time::milliseconds(i * 500));
                            timer.async_wait(yield[ec]);

                            if (ec)
                                break;
                        }
 
                        m_socket.async_connect(socket.u8string(), yield[ec]);
#ifdef WIN32
                        if (ec.value() != WSAECONNREFUSED)
#else
                        if (ec.value() != boost::system::errc::no_such_file_or_directory && ec.value() != boost::system::errc::connection_refused)
#endif
                        {
                            break;
                        }
                    }

                    if (ec)
                        throw pipe_error("Can't connect to " + socket.string() + " due the error \'" + ec.message() + "\'");
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

            void adjust() noexcept(false) override
            {
                perform(message::make(message::adjust));
            }

            void status(std::vector<slipway::health>& result) noexcept(false) override
            {
                result = perform<std::vector<slipway::health>>(message::make(message::status));
            }

            void review(std::vector<slipway::report>& result) noexcept(false) override
            {
                result = perform<std::vector<slipway::report>>(message::make(message::review));
            }

            void unplug(const handle& service) noexcept(false) override
            {
                perform(message::make(message::unplug, service));
            }

            void engage(const handle& service) noexcept(false) override
            {
                perform(message::make(message::engage, service));
            }

            void adjust(const handle& service) noexcept(false) override
            {
                perform(message::make(message::adjust, service));
            }

            void status(const handle& service, slipway::health& result) noexcept(false) override
            {
                result = perform<slipway::health>(message::make(message::status, service));
            }

            void review(const handle& service, slipway::report& result) noexcept(false) override
            {
                result = perform<slipway::report>(message::make(message::review, service));
            }
        };
    }

    std::shared_ptr<backend> connect_backend(const std::string& home) noexcept(false)
    {
        return std::make_shared<client>(home);
    }
}
