#include "message.h"
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef WIN32
    #include <windows.h>
#endif

namespace slipway
{
    class engine
    {
        boost::asio::io_context& m_io;

        void adjust() noexcept(false)
        {
        }

        void adjust(const slipway::handle& service) noexcept(false)
        {
        }

        void unplug() noexcept(false)
        {
        }

        void unplug(const slipway::handle& service) noexcept(false)
        {
        }

        void engage() noexcept(false)
        {
        }

        void engage(const slipway::handle& service) noexcept(false)
        {
        }
 
        void reboot() noexcept(false)
        {
        }

        void reboot(const slipway::handle& service) noexcept(false)
        {
        }

        std::vector<slipway::health> status() noexcept(false)
        {
            return {};
        }

        slipway::health status(const slipway::handle& service) noexcept(false)
        {
            return {};
        }

        std::vector<slipway::report> report() noexcept(false)
        {
            return {};
        }

        slipway::report report(const slipway::handle& service) noexcept(false)
        {
            return {};
        }

    public:

        engine(boost::asio::io_context& io, const std::string& home)
            : m_io(io)
        {

        }

        void handle_request(boost::asio::streambuf& buffer) noexcept(true)
        {
            slipway::message req, res;

            try
            {
                slipway::pull_message(buffer, req);
                switch (req.action)
                {
                    case slipway::message::adjust:
                    {
                        req.payload.index() == 1 
                            ? adjust(std::get<slipway::handle>(req.payload)) 
                            : adjust();
                        res = slipway::message::make(slipway::message::adjust);
                        break;
                    }
                    case slipway::message::unplug:
                    {
                        req.payload.index() == 1 
                            ? unplug(std::get<slipway::handle>(req.payload)) 
                            : unplug();
                        res = slipway::message::make(slipway::message::unplug);
                        break;
                    }
                    case slipway::message::engage:
                    {
                        req.payload.index() == 1 
                            ? engage(std::get<slipway::handle>(req.payload)) 
                            : engage();
                        res = slipway::message::make(slipway::message::engage);
                        break;
                    }
                    case slipway::message::reboot:
                    {
                        req.payload.index() == 1 
                            ? reboot(std::get<slipway::handle>(req.payload)) 
                            : reboot();
                        res = slipway::message::make(slipway::message::reboot);
                        break;
                    }
                    case slipway::message::status:
                    {
                        res = req.payload.index() == 1
                            ? slipway::message::make(slipway::message::status, status(std::get<slipway::handle>(req.payload)))
                            : slipway::message::make(slipway::message::status, status());
                        break;
                    }
                    case slipway::message::review:
                    {
                        res = req.payload.index() == 1
                            ? slipway::message::make(slipway::message::review, report(std::get<slipway::handle>(req.payload)))
                            : slipway::message::make(slipway::message::review, report());
                        break;
                    }
                    default:
                        res = slipway::message::make(req.action, "wrong command");
                        break;
                }
            }
            catch (const std::exception& ex)
            {
                res = slipway::message::make(req.action, ex.what());
            }

            slipway::push_message(buffer, req);
        }
    };

    class server
    {
        boost::asio::io_context& m_io;
        boost::asio::local::stream_protocol::acceptor m_acceptor;
        slipway::engine m_engine;

        void handle(boost::asio::local::stream_protocol::socket client)
        {
            boost::asio::spawn(m_io, [this, socket = std::move(client)](boost::asio::yield_context yield) mutable
            {
                boost::asio::streambuf buffer;
                boost::system::error_code error;

                boost::asio::async_read_until(socket, buffer, '\n', yield[error]);
                if (error)
                {
                    if (error != boost::asio::error::operation_aborted)
                        std::cerr << error.message() << std::endl;

                    error = socket.close(error);
                    return;
                }

                m_engine.handle_request(buffer);

                boost::asio::async_write(socket, buffer, yield[error]);
                if (error)
                {
                    if (error != boost::asio::error::operation_aborted)
                        std::cerr << error.message() << std::endl;

                    error = socket.close(error);
                    return;
                }

                handle(std::move(socket));
            });
        }

        void accept()
        {
            m_acceptor.async_accept([this](boost::system::error_code ec, boost::asio::local::stream_protocol::socket socket)
            {
                if (ec)
                {
                    if (ec != boost::asio::error::operation_aborted)
                        throw boost::system::system_error(ec, ec.message());

                    m_io.stop();
                    return;
                }

                handle(std::move(socket));
                accept();
            });
        }

    public:

        server(boost::asio::io_context& io, const boost::asio::local::stream_protocol::endpoint& ep, const std::string& home)
            : m_io(io)
            , m_acceptor(io, ep.protocol())
            , m_engine(io, home)
        {
            m_acceptor.bind(ep);
            m_acceptor.listen();

            accept();
        }
    };
}

int main(int argc, char* argv[])
{
    try
    {
        std::filesystem::path home = std::filesystem::path(argv[1]);
        std::filesystem::path socket = home / "slipway.jack";
        std::filesystem::path locker = home / "slipway.lock";

        if (!std::filesystem::exists(locker))
            std::ofstream(locker).close();

        boost::interprocess::file_lock guard(locker.string().c_str());
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(guard, boost::interprocess::try_to_lock_type());

        if (!lock.owns())
        {
            std::cerr << "can't acquire lock" << std::endl;
            return 1;
        }

#ifdef WIN32
        DeleteFile(socket.string().c_str());
#else
        ::unlink(socket.string().c_str());
#endif

        boost::asio::io_context io;
        slipway::server server(io, socket.string(), home.string());
        io.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 2;
    }

    return 0;
}
