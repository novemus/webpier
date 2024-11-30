#include "client.h"
#include "../utils.h"

namespace slipway
{
    constexpr const char* req_pipe_name = "slipway.0";
    constexpr const char* res_pipe_name = "slipway.1";
    constexpr const char* lock_file_name = "lock";

    class client_impl : public client
    {

    };
}
