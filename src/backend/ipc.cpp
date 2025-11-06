#include <backend/ipc.h>

namespace slipway { namespace ipc {
#ifdef WIN32
    slipway::ipc::endpoint make_endpoint(const std::filesystem::path& home) noexcept(true)
    {
        auto base = std::filesystem::path("\\\\.\\pipe") / std::to_string(std::hash<std::string>()(home.string()));
        return slipway::ipc::endpoint(base.string() + slipway_socket_ext);
    }

    void connect_server(slipway::ipc::socket& socket, const slipway::ipc::endpoint& server, boost::system::error_code& ec) noexcept(true)
    {
        ec.clear();
        if (WaitNamedPipeA(server.path().c_str(), 500))
        {
            auto pipe = CreateFileA(server.path().c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
            if (pipe != INVALID_HANDLE_VALUE) 
            {
                socket.assign(pipe);
                return;
            }
        }
        ec.assign(GetLastError(), boost::system::system_category());
    }
#else
    slipway::ipc::endpoint make_endpoint(const std::filesystem::path& home) noexcept(true)
    {
        auto base = std::filesystem::temp_directory_path() / std::to_string(std::hash<std::string>()(home.string()));
        return slipway::ipc::endpoint(base.string() + slipway_socket_ext);
    }

    void connect_server(slipway::ipc::socket& socket, const slipway::ipc::endpoint& server, boost::system::error_code& ec) noexcept(true)
    {
        ec.clear();
        socket.connect(server.path(), ec);
    }
#endif

}}