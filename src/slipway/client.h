#pragma once

#include "message.h"
#include <string>
#include <memory>

namespace slipway
{
    struct client
    {
        virtual ~client() {}
        virtual void launch() noexcept(false) = 0;
        virtual void finish() noexcept(false) = 0;
        virtual void reboot() noexcept(false) = 0;
        virtual void status(std::vector<outline>& result) noexcept(false) = 0;
        virtual void report(std::vector<snapshot>& result) noexcept(false) = 0;
        virtual void launch(const handle& service) noexcept(false) = 0;
        virtual void finish(const handle& service) noexcept(false) = 0;
        virtual void reboot(const handle& service) noexcept(false) = 0;
        virtual void status(const handle& service, outline& result) noexcept(false) = 0;
        virtual void report(const handle& service, snapshot& result) noexcept(false) = 0;
    };

    std::shared_ptr<client> open_client(const std::string& context) noexcept(false);
}
