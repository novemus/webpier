#include <boost/program_options.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/lexical_cast.hpp>
#include <wormhole/wormhole.h>
#include <wormhole/logger.h>
#include <store/utils.h>

template<class endpoint>
endpoint parse_endpoint(const std::string& str)
{
    size_t delim = str.find_last_of(':');
    return endpoint(
        boost::asio::ip::address::from_string(str.substr(0, delim)), 
        boost::lexical_cast<uint16_t>(str.substr(delim + 1))
    );
}

int main(int argc, char *argv[])
{
    if(webpier::get_module_path(webpier::carrier_module) != std::filesystem::canonical(argv[0]))
    {
        std::cerr << "wrong module path" << std::endl;
        return 1;
    }

    boost::program_options::options_description options;
    options.add_options()
        ("purpose", boost::program_options::value<std::string>()->required())
        ("service", boost::program_options::value<std::string>()->required())
        ("gateway", boost::program_options::value<std::string>()->required())
        ("faraway", boost::program_options::value<std::string>()->required())
        ("obscure", boost::program_options::value<uint64_t>()->default_value(0))
        ("journal", boost::program_options::value<std::string>()->default_value(""))
        ("logging", boost::program_options::value<wormhole::log::severity>()->default_value(wormhole::log::info));

    try
    {
        boost::program_options::variables_map config;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), config);
        boost::program_options::notify(config);

        wormhole::log::set(config["logging"].as<wormhole::log::severity>(), config["journal"].as<std::string>());

        auto purpose = config["purpose"].as<std::string>();
        auto service = parse_endpoint<boost::asio::ip::tcp::endpoint>(config["service"].as<std::string>());
        auto gateway = parse_endpoint<boost::asio::ip::udp::endpoint>(config["gateway"].as<std::string>());
        auto faraway = parse_endpoint<boost::asio::ip::udp::endpoint>(config["faraway"].as<std::string>());
        auto obscure = config["obscure"].as<uint64_t>();

        _inf_ << "starting tunnel for purpose=" << purpose << " service=" << service << " gateway=" << gateway << " faraway=" << faraway << " obscure=" << (obscure != 0);

        boost::asio::io_service io;
        auto router = purpose == "import"
                    ? wormhole::create_importer(io, service, gateway, faraway, obscure)
                    : wormhole::create_exporter(io, service, gateway, faraway, obscure);

        router->launch();
        io.run();
    }
    catch (const std::exception& e)
    {
        _ftl_ << e.what();
        return 2;
    }

    return 0;
}
