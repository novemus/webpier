#include <backend/ipc.h>
#include <store/utils.h>

namespace slipway { namespace ipc {
#ifdef WIN32

    DWORD fill_security_attributes(SECURITY_ATTRIBUTES* sa)
    {
        PSECURITY_DESCRIPTOR sd = NULL;
        HANDLE token;
        PTOKEN_USER user = NULL;
        DWORD length = 0;

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
            return GetLastError();

        if (!GetTokenInformation(token, TokenUser, NULL, 0, &length))
        {
            DWORD error = GetLastError();
            if (error != ERROR_INSUFFICIENT_BUFFER)
            {
                DWORD error = GetLastError();
                CloseHandle(token);
                return error;
            }
        }

        user = (PTOKEN_USER)LocalAlloc(LPTR, length);
        if (user == NULL)
        {
            DWORD error = GetLastError();
            CloseHandle(token);
            return error;
        }

        LPSTR sid = NULL;
        if (!GetTokenInformation(token, TokenUser, user, length, &length) || !ConvertSidToStringSidA(user->User.Sid, &sid))
        {
            DWORD error = GetLastError();
            if (sid)
                LocalFree(sid);
            LocalFree(user);
            CloseHandle(token);
            return error;
        }

        std::string sddl;
        sddl.append("D:(A;;GA;;;").append(sid).append(")S:(ML;;NW;;;LW)");

        LocalFree(sid);
        LocalFree(user);
        CloseHandle(token);

        if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(sddl.c_str(), SDDL_REVISION_1, &sd, NULL))
            return GetLastError();

        ZeroMemory(sa, sizeof(SECURITY_ATTRIBUTES));
        sa->nLength = sizeof(SECURITY_ATTRIBUTES);
        sa->lpSecurityDescriptor = sd;
        sa->bInheritHandle = FALSE;

        return ERROR_SUCCESS;
    }

    slipway::ipc::endpoint make_endpoint(const std::filesystem::path& home) noexcept(true)
    {
        auto base = std::filesystem::path("\\\\.\\pipe") / webpier::make_text_hash(home.string());
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
        auto temp = std::filesystem::exists("/tmp") ? std::filesystem::path("/tmp") : std::filesystem::temp_directory_path();
        auto base = temp / webpier::make_text_hash(home.string());
        return slipway::ipc::endpoint(base.string() + slipway_socket_ext);
    }

    void connect_server(slipway::ipc::socket& socket, const slipway::ipc::endpoint& server, boost::system::error_code& ec) noexcept(true)
    {
        ec.clear();
        socket.connect(server.path(), ec);
    }

#endif
}}