#pragma once

#include "channel.h"
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
        virtual void status(std::vector<slipway::wealth>& result) noexcept(false) = 0;
        virtual void review(std::vector<slipway::report>& result) noexcept(false) = 0;
        virtual void launch(const slipway::handle& service) noexcept(false) = 0;
        virtual void finish(const slipway::handle& service) noexcept(false) = 0;
        virtual void reboot(const slipway::handle& service) noexcept(false) = 0;
        virtual void status(const slipway::handle& service, slipway::wealth& result) noexcept(false) = 0;
        virtual void review(const slipway::handle& service, slipway::report& result) noexcept(false) = 0;
    };

    std::shared_ptr<client> open_client(const std::string& context) noexcept(false);
}
