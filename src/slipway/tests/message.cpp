#include "../message.h"
#include <boost/asio.hpp>
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
    BOOST_CHECK_EQUAL(std::get<0>(initial.payload), "error");

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(!replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK_EQUAL(std::get<0>(replica.payload), std::get<0>(initial.payload));

    slipway::handle ident { 
        "someone@mail.box/pier",
        "service" 
        };

    initial = slipway::message::make(slipway::message::finish, ident);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::finish);
    BOOST_CHECK_EQUAL(initial.payload.index(), 1);
    BOOST_CHECK(std::get<1>(initial.payload) == ident);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<1>(replica.payload) == std::get<1>(initial.payload));

    slipway::outline state { 
        ident,
        slipway::outline::active
        };

    initial = slipway::message::make(slipway::message::status, state);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::status);
    BOOST_CHECK_EQUAL(initial.payload.index(), 2);
    BOOST_CHECK(std::get<2>(initial.payload) == state);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<2>(replica.payload) == std::get<2>(initial.payload));

    slipway::snapshot report {
        state,
        "/path/to/service/log/file",
        {
            { "someone@mail.box/pier", "/path/to/someone/log/file", 1 },
            { "someoneelse@mail.box/pier", "/path/to/someoneelse/log/file", 2 }
        }
    };

    initial = slipway::message::make(slipway::message::report, report);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::report);
    BOOST_CHECK_EQUAL(initial.payload.index(), 3);
    BOOST_CHECK(std::get<3>(initial.payload) == report);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<3>(replica.payload) == std::get<3>(initial.payload));

    std::vector<slipway::outline> states { state, state };

    initial = slipway::message::make(slipway::message::status, states);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::status);
    BOOST_CHECK_EQUAL(initial.payload.index(), 4);
    BOOST_CHECK(std::get<4>(initial.payload) == states);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<4>(replica.payload) == std::get<4>(initial.payload));

    std::vector<slipway::snapshot> reports { report, report };

    initial = slipway::message::make(slipway::message::report, reports);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::report);
    BOOST_CHECK_EQUAL(initial.payload.index(), 5);
    BOOST_CHECK(std::get<5>(initial.payload) == reports);

    BOOST_REQUIRE_NO_THROW(slipway::put_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::get_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<5>(replica.payload) == std::get<5>(initial.payload));
}
