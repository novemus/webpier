#include <filesystem>
#include <boost/process.hpp>
#include <boost/scope_exit.hpp>
#include <boost/test/unit_test.hpp>
#include <backend/client.h>
#include <store/context.h>
#include <store/utils.h>

BOOST_AUTO_TEST_CASE(client)
{
    std::string host = "host@mail.box/test";
    std::string peer = "peer@mail.box/test";

    auto home = std::filesystem::current_path() / std::to_string(std::time(0));
    auto repo = home / webpier::to_hexadecimal(host.data(), host.size());

    BOOST_REQUIRE_NO_THROW(std::filesystem::create_directory(home));

    BOOST_SCOPE_EXIT(&home)
    {
        std::filesystem::remove_all(home);
    } 
    BOOST_SCOPE_EXIT_END

    auto context = webpier::open_context(home.string());
    webpier::config conf { host, repo.string(), { home.string(), webpier::journal::trace }, {}, { webpier::default_dht_bootstrap, webpier::default_dht_port, 0 }, {} };

    BOOST_REQUIRE_NO_THROW(context->set_config(conf));
    BOOST_REQUIRE_NO_THROW(context->add_pier(peer, webpier::load_x509_cert(home / repo / host / "cert.crt")));

    webpier::service foo { true, "foo", peer, "127.0.0.1:1234", webpier::default_gateway, webpier::default_dht_bootstrap, true, false };
    webpier::service bar { true, "bar", peer, "127.0.0.1:5678", webpier::default_gateway, webpier::default_dht_bootstrap, true, false };

    BOOST_REQUIRE_NO_THROW(context->add_export_service(foo));

    boost::process::child server(webpier::get_module_path(SLIPWAY_MODULE).string(), home.string());
    auto client = slipway::connect_backend(home.string());

    slipway::handle foo_handle { host , "foo" };
    slipway::health foo_asleep { foo_handle, slipway::health::asleep };
    slipway::health foo_active { foo_handle, slipway::health::lonely };

    slipway::handle bar_handle { host , "bar" };
    slipway::health bar_asleep { bar_handle, slipway::health::asleep };
    slipway::health bar_active { bar_handle, slipway::health::lonely };

    std::vector<slipway::health> result;
    BOOST_REQUIRE_NO_THROW(client->status(result));
    BOOST_REQUIRE_EQUAL(result.size(), 1);
    BOOST_CHECK(result[0] == foo_active);

    slipway::health health;
    BOOST_REQUIRE_NO_THROW(client->status(foo_handle, health));
    BOOST_CHECK(health == foo_active);

    BOOST_REQUIRE_NO_THROW(client->unplug(foo_handle));
    BOOST_REQUIRE_NO_THROW(client->status(foo_handle, health));
    BOOST_CHECK(health == foo_asleep);

    BOOST_REQUIRE_NO_THROW(context->add_export_service(bar));

    BOOST_REQUIRE_NO_THROW(client->engage(bar_handle));
    BOOST_REQUIRE_NO_THROW(client->status(result));
    BOOST_REQUIRE_EQUAL(result.size(), 2);
    BOOST_CHECK(result[0] == bar_active);
    BOOST_CHECK(result[1] == foo_asleep);

    BOOST_REQUIRE_NO_THROW(client->engage());
    BOOST_REQUIRE_NO_THROW(client->status(result));
    BOOST_REQUIRE_EQUAL(result.size(), 2);
    BOOST_CHECK(result[0] == bar_active);
    BOOST_CHECK(result[1] == foo_active);

    BOOST_REQUIRE_NO_THROW(client->unplug());
    BOOST_REQUIRE_NO_THROW(client->status(result));
    BOOST_REQUIRE_EQUAL(result.size(), 2);
    BOOST_CHECK(result[0] == bar_asleep);
    BOOST_CHECK(result[1] == foo_asleep);

    BOOST_REQUIRE_NO_THROW(context->del_export_service("foo"));

    BOOST_REQUIRE_NO_THROW(client->engage());
    BOOST_REQUIRE_NO_THROW(client->status(result));
    BOOST_REQUIRE_EQUAL(result.size(), 1);
    BOOST_CHECK(result[0] == bar_active);

    BOOST_REQUIRE_NO_THROW(client.reset());
    BOOST_REQUIRE_NO_THROW(server.join());
}
