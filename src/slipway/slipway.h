#pragma once

#include "message.h"
#include <string>
#include <memory>

namespace slipway
{
    struct daemon
    {
        virtual ~daemon() {}
        // suspend all services
        virtual void unplug() noexcept(false) = 0;
        // reset all services according to its configurations and drop removed ones
        virtual void engage() noexcept(false) = 0;
        // soft restart active services, boot new ones according to its configurations and drop removed ones
        virtual void adjust() noexcept(false) = 0;
        // get actual status of all services
        virtual void status(std::vector<slipway::health>& result) noexcept(false) = 0;
        // get reports on all service
        virtual void review(std::vector<slipway::report>& result) noexcept(false) = 0;
        // suspend the specified service
        virtual void unplug(const slipway::handle& service) noexcept(false) = 0;
        // start or restart the specified service
        virtual void engage(const slipway::handle& service) noexcept(false) = 0;
        // restart active service or boot new one according to it configuration
        virtual void adjust(const slipway::handle& service) noexcept(false) = 0;
        // get actual status of the specified service
        virtual void status(const slipway::handle& service, slipway::health& result) noexcept(false) = 0;
        // get a report on the specified service
        virtual void review(const slipway::handle& service, slipway::report& result) noexcept(false) = 0;
    };

    std::shared_ptr<daemon> create_client(const std::string& home) noexcept(false);
}
