#pragma once

#include "message.h"
#include <string>
#include <memory>

namespace slipway
{
    struct daemon
    {
        virtual ~daemon() {}
        // shutdown all services
        virtual void unplug() noexcept(false) = 0;
        // adjust services according to the configuration
        virtual void engage() noexcept(false) = 0;
        // reboot of all services according to the configuration
        virtual void reboot() noexcept(false) = 0;
        // get actual status of all services
        virtual void status(std::vector<slipway::health>& result) noexcept(false) = 0;
        // get extended info about all services
        virtual void review(std::vector<slipway::report>& result) noexcept(false) = 0;
        // shutdown the specified service
        virtual void unplug(const slipway::handle& service) noexcept(false) = 0;
        // adjust the specified service according to the configuration
        virtual void engage(const slipway::handle& service) noexcept(false) = 0;
        // reboot the specified service
        virtual void reboot(const slipway::handle& service) noexcept(false) = 0;
        // get actual status of the specified service
        virtual void status(const slipway::handle& service, slipway::health& result) noexcept(false) = 0;
        // get extended info about the specified service
        virtual void review(const slipway::handle& service, slipway::report& result) noexcept(false) = 0;
    };

    std::shared_ptr<daemon> create_client(const std::string& home) noexcept(false);
}
