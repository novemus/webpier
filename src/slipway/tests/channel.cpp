#include "../channel.h"
#include <filesystem>
#include <thread>
#include <boost/asio.hpp>
#include <boost/scope_exit.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(message)
{
    slipway::message initial = slipway::message::make(slipway::message::reboot), replica;
    boost::asio::streambuf buffer;

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::reboot);
    BOOST_CHECK_EQUAL(initial.payload.index(), 0);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());

    initial = slipway::message::make(slipway::message::launch, "error");

    BOOST_CHECK(!initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::launch);
    BOOST_CHECK_EQUAL(initial.payload.index(), 0);
    BOOST_CHECK_EQUAL(std::get<std::string>(initial.payload), "error");

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(!replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK_EQUAL(std::get<std::string>(replica.payload), std::get<std::string>(initial.payload));

    slipway::handle ident { 
        "someone@mail.box/pier",
        "service" 
        };
    initial = slipway::message::make(slipway::message::finish, ident);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::finish);
    BOOST_CHECK_EQUAL(initial.payload.index(), 1);
    BOOST_CHECK(std::get<slipway::handle>(initial.payload) == ident);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<slipway::handle>(replica.payload) == std::get<slipway::handle>(initial.payload));

    slipway::wealth state { 
        ident,
        slipway::wealth::active
        };
    initial = slipway::message::make(slipway::message::status, state);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::status);
    BOOST_CHECK_EQUAL(initial.payload.index(), 2);
    BOOST_CHECK(std::get<slipway::wealth>(initial.payload) == state);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<slipway::wealth>(replica.payload) == std::get<slipway::wealth>(initial.payload));

    slipway::report report {
        state,
        "/path/to/service/log/file",
        {
            { "someone@mail.box/pier", "/path/to/someone/log/file", 1 },
            { "someoneelse@mail.box/pier", "/path/to/someoneelse/log/file", 2 }
        }
    };
    initial = slipway::message::make(slipway::message::review, report);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::review);
    BOOST_CHECK_EQUAL(initial.payload.index(), 3);
    BOOST_CHECK(std::get<slipway::report>(initial.payload) == report);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<slipway::report>(replica.payload) == std::get<slipway::report>(initial.payload));

    std::vector<slipway::wealth> states { state, state };
    initial = slipway::message::make(slipway::message::status, states);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::status);
    BOOST_CHECK_EQUAL(initial.payload.index(), 4);
    BOOST_CHECK(std::get<std::vector<slipway::wealth>>(initial.payload) == states);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<std::vector<slipway::wealth>>(replica.payload) == std::get<std::vector<slipway::wealth>>(initial.payload));

    std::vector<slipway::report> reports { report, report };
    initial = slipway::message::make(slipway::message::review, reports);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::review);
    BOOST_CHECK_EQUAL(initial.payload.index(), 5);
    BOOST_CHECK(std::get<std::vector<slipway::report>>(initial.payload) == reports);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<std::vector<slipway::report>>(replica.payload) == std::get<std::vector<slipway::report>>(initial.payload));
}


BOOST_AUTO_TEST_CASE(channel)
{
    auto dest = std::filesystem::current_path() / std::to_string(std::time(0));
    
    auto nil = dest / "slipway.0";
    auto one = dest / "slipway.1";

    BOOST_SCOPE_EXIT(&dest) 
    {
        std::filesystem::remove_all(dest);
    } 
    BOOST_SCOPE_EXIT_END

    BOOST_REQUIRE(std::filesystem::create_directory(dest));

    slipway::handle ident { 
        "someone@mail.box/pier",
        "service" 
        };

    slipway::wealth state { 
        ident,
        slipway::wealth::active
        };

    std::shared_ptr<slipway::channel> server = slipway::create_channel(one.string(), nil.string());
    std::shared_ptr<slipway::channel> client = slipway::create_channel(nil.string(), one.string());

    auto s = std::async(std::launch::async, [&]()
    {
        slipway::message data;

        BOOST_REQUIRE_NO_THROW(server->pull(data));
        BOOST_CHECK_EQUAL(data.action, slipway::message::launch);
        BOOST_CHECK(data.ok());
        BOOST_REQUIRE_NO_THROW(server->push(slipway::message::make(slipway::message::launch)));

        BOOST_REQUIRE_NO_THROW(server->pull(data));
        BOOST_CHECK_EQUAL(data.action, slipway::message::reboot);
        BOOST_CHECK(data.ok());
        BOOST_REQUIRE_NO_THROW(server->push(slipway::message::make(slipway::message::reboot, "error")));

        BOOST_REQUIRE_NO_THROW(server->pull(data));
        BOOST_CHECK_EQUAL(data.action, slipway::message::finish);
        BOOST_CHECK(data.ok());
        BOOST_CHECK(std::get<slipway::handle>(data.payload) == ident);
        BOOST_REQUIRE_NO_THROW(server->push(slipway::message::make(slipway::message::finish)));

        BOOST_REQUIRE_NO_THROW(server->pull(data));
        BOOST_CHECK_EQUAL(data.action, slipway::message::status);
        BOOST_CHECK(data.ok());
        BOOST_CHECK(std::get<slipway::handle>(data.payload) == ident);
        BOOST_REQUIRE_NO_THROW(server->push(slipway::message::make(slipway::message::status, state)));
    });

    auto c = std::async(std::launch::async, [&]()
    {
        slipway::message data;

        BOOST_REQUIRE_NO_THROW(client->push(slipway::message::make(slipway::message::launch)));
        BOOST_REQUIRE_NO_THROW(client->pull(data));
        BOOST_CHECK_EQUAL(data.action, slipway::message::launch);
        BOOST_CHECK(data.ok());

        BOOST_REQUIRE_NO_THROW(client->push(slipway::message::make(slipway::message::reboot)));
        BOOST_REQUIRE_NO_THROW(client->pull(data));
        BOOST_CHECK_EQUAL(data.action, slipway::message::reboot);
        BOOST_CHECK(!data.ok());
        BOOST_CHECK_EQUAL(std::get<std::string>(data.payload), "error");

        BOOST_REQUIRE_NO_THROW(client->push(slipway::message::make(slipway::message::finish, ident)));
        BOOST_REQUIRE_NO_THROW(client->pull(data));
        BOOST_CHECK_EQUAL(data.action, slipway::message::finish);
        BOOST_CHECK(data.ok());

        BOOST_REQUIRE_NO_THROW(client->push(slipway::message::make(slipway::message::status, ident)));
        BOOST_REQUIRE_NO_THROW(client->pull(data));
        BOOST_CHECK_EQUAL(data.action, slipway::message::status);
        BOOST_CHECK(data.ok());
        BOOST_CHECK(std::get<slipway::wealth>(data.payload) == state);
    });

    s.wait();
    c.wait();
}
