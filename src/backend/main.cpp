#include <backend/server.h>
#include <store/utils.h>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <fstream>


int main(int argc, char* argv[])
{
    constexpr const char* slipway_lock_file_name = "slipway.lock";
    constexpr const char* slipway_jack_file_name = "slipway.jack";

    try
    {
        if(webpier::get_module_path(webpier::slipway_module) != std::filesystem::canonical(argv[0]))
        {
            std::cerr << "wrong module path" << std::endl;
            return 1;
        }

        if (argc < 2)
        {
            std::cerr << "no home argument" << std::endl;
            return 2;
        }

        bool steady = argc == 3 && std::strcmp(argv[2], "daemon") == 0;

        std::filesystem::path home = std::filesystem::path(argv[1]);
        if (!std::filesystem::exists(home))
        {
            std::cerr << "wrong home argument" << std::endl;
            return 3;
        }

        std::filesystem::path socket = home / slipway_jack_file_name;
        std::filesystem::path locker = home / slipway_lock_file_name;

        if (!std::filesystem::exists(locker))
            std::ofstream(locker).close();

        boost::interprocess::file_lock guard(locker.string().c_str());
        boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(guard, boost::interprocess::try_to_lock_type());

        if (!lock.owns())
        {
            std::cerr << "can't acquire lock" << std::endl;
            return 4;
        }

        auto server = slipway::create_backend(home.string(), steady);
        server->employ();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 5;
    }

    return 0;
}
