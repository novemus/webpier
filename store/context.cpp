#include "context.h"

namespace webpier
{
    class context_impl : public context
    {
        struct
        {
            std::string config;
            std::string repo;
            std::string logs;
        } paths;

        options basis;
        std::vector<service> locals;
        std::vector<service> remotes;

    public:

        context_impl(const std::string& config)
        {
            paths.config = config;
        }

        ~context_impl() override
        {
        }

        void renew() noexcept(false) override
        {

        }

        void flush(bool force = false) const noexcept(false) override
        {

        }

        void get_options(options& data) const noexcept(true) override
        {

        }

        void set_options(const options& data) noexcept(true) override
        {

        }

        void get_local_services(std::vector<service>& data) const noexcept(true) override
        {

        }

        void set_local_services(const std::vector<service>& data) noexcept(true) override
        {

        }

        void del_local_service(const service& data) noexcept(true) override
        {

        }

        void set_local_service(const service& data) noexcept(true) override
        {

        }

        void get_remote_services(std::vector<service>& data) const noexcept(true) override
        {

        }

        void set_remote_services(const std::vector<service>& data) noexcept(true) override
        {

        }

        void del_remote_service(const service& data) noexcept(true) override
        {

        }

        void set_remote_service(const service& data) noexcept(true) override
        {

        }
    };

    std::shared_ptr<context> open_context(const std::string& config)
    {
        return std::make_shared<context_impl>(config);
    }
}
