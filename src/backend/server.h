#pragma once

#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <filesystem>

namespace slipway
{
    struct server
    {
        virtual ~server() {}
        // synchronous execution
        virtual void employ() noexcept(false) = 0;
        // thread-safe cancellation
        virtual void cancel() noexcept(true) = 0;
    };

    // home - path to the webpier context directory
    // steady - pass true to force the endless server running, otherwise the server
    // will run at least for one client session until the last session is destroyed
    std::shared_ptr<server> create_backend(boost::asio::io_context& io, const std::filesystem::path& home, bool steady) noexcept(false);
}
