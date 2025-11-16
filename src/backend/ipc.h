#pragma once

#include <string>
#include <filesystem>

constexpr const char* slipway_socket_ext = ".slipway";

#ifdef WIN32

#include <memory>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/windows/stream_handle.hpp>
#include <boost/system/error_code.hpp>
#include <windows.h>
#include <aclapi.h>
#include <sddl.h>

namespace slipway { namespace ipc {

class endpoint
{
    std::string m_path;

public:

    explicit endpoint(const std::string& path = {}) 
        : m_path(path)
    {}

    const std::string& path() const
    {
        return m_path;
    }
};

class protocol {};
using socket = boost::asio::windows::stream_handle;

class acceptor
{
    boost::asio::io_context& m_io;
    slipway::ipc::endpoint m_endpoint;
    boost::asio::windows::overlapped_ptr m_overlapped;

public:

    explicit acceptor(boost::asio::io_context& io)
        : m_io(io)
    {
    }

    acceptor(boost::asio::io_context& io, const slipway::ipc::protocol&)
        : m_io(io)
    {
    }

    acceptor(boost::asio::io_context& io, const slipway::ipc::endpoint& ep)
        : m_io(io)
        , m_endpoint(ep)
    {
    }

    acceptor(const acceptor&) = delete;
    acceptor& operator=(const acceptor&) = delete;

    ~acceptor()
    {
        cancel();
    }

    slipway::ipc::endpoint local_endpoint() const noexcept(true)
    {
        return m_endpoint;
    }

    slipway::ipc::endpoint local_endpoint(boost::system::error_code& ec) const noexcept(true)
    {
        return m_endpoint;
    }

    void bind(const slipway::ipc::endpoint& ep) noexcept(true)
    {
        m_endpoint = ep;
    }

    void bind(const slipway::ipc::endpoint& ep, boost::system::error_code& ec) noexcept(true)
    {
        m_endpoint = ep;
    }

    void close() noexcept(true)
    {
        cancel();
    }

    void close(boost::system::error_code& ec) noexcept(true)
    {
        cancel();
    }

    void cancel() noexcept(true)
    {
        m_overlapped.reset();
    }

    void cancel(boost::system::error_code& ec) noexcept(true)
    {
        m_overlapped.reset();
    }

    void listen() noexcept(true)
    {
    }

    void listen(boost::system::error_code& ec) noexcept(true)
    {
    }

    slipway::ipc::socket accept() const noexcept(false)
    {
        boost::system::error_code ec;
        boost::asio::windows::stream_handle stream = accept(ec);
        if (ec)
            throw boost::system::system_error(ec);

        return std::move(stream);
    }

    slipway::ipc::socket accept(boost::system::error_code& ec) const noexcept(true)
    {
        auto pipe = CreateNamedPipeA(
            m_endpoint.path().c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            512,
            512,
            0,
            nullptr
        );

        slipway::ipc::socket stream(m_io);
        stream.assign(pipe);

        if (pipe == INVALID_HANDLE_VALUE || !ConnectNamedPipe(pipe, NULL))
            ec.assign(GetLastError(), boost::system::system_category());

        return std::move(stream);
    }

    void async_accept(std::function<void(const boost::system::error_code&, slipway::ipc::socket)>&& handler) noexcept(true)
    {
        PSECURITY_DESCRIPTOR sd = NULL;
        SECURITY_ATTRIBUTES sa;

        LPCSTR sddl = "D:(A;;GA;;;WD)S:(ML;;NW;;;LW)";

        if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(sddl, SDDL_REVISION_1, &sd, NULL))
        {
            m_overlapped.complete(boost::system::error_code(GetLastError(), boost::system::system_category()), 0);
            return;
        }

        ZeroMemory(&sa, sizeof(sa));
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = sd;
        sa.bInheritHandle = FALSE;

        auto pipe = CreateNamedPipeA(
            m_endpoint.path().c_str(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            512,
            512,
            0,
            &sa
        );

        slipway::ipc::socket stream(m_io);
        stream.assign(pipe);

        m_overlapped.reset(m_io, [socket = std::move(stream), handler](const boost::system::error_code& ec, size_t) mutable
        {
            handler(ec, std::move(socket));
        });

        if (pipe == INVALID_HANDLE_VALUE)
        {
            m_overlapped.complete(boost::system::error_code(GetLastError(), boost::system::system_category()), 0);
            return;
        }

        if (ConnectNamedPipe(pipe, m_overlapped.get()))
        {
            m_overlapped.complete(boost::system::error_code(), 0);
        } 
        else
        {
            DWORD error = GetLastError();
            if (error == ERROR_IO_PENDING)
            {
                m_overlapped.release(); 
            } 
            else if (error == ERROR_PIPE_CONNECTED)
            {
                m_overlapped.complete(boost::system::error_code(), 0);
            }
            else
            {
                m_overlapped.complete(boost::system::error_code(error, boost::system::system_category()), 0);
            }
        }
    }
};

slipway::ipc::endpoint make_endpoint(const std::filesystem::path& home) noexcept(true);
void connect_server(slipway::ipc::socket& client, const slipway::ipc::endpoint& server, boost::system::error_code& ec) noexcept(true);

}}

#else

#include <boost/asio/local/stream_protocol.hpp>

namespace slipway { namespace ipc {

using protocol = boost::asio::local::stream_protocol;
using endpoint = boost::asio::local::stream_protocol::endpoint;
using socket = boost::asio::local::stream_protocol::socket;
using acceptor = boost::asio::local::stream_protocol::acceptor;

slipway::ipc::endpoint make_endpoint(const std::filesystem::path& home) noexcept(true);
void connect_server(slipway::ipc::socket& client, const slipway::ipc::endpoint& server, boost::system::error_code& ec) noexcept(true);

}}

#endif
