#include "../message.h"
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(message)
{
    slipway::message initial = slipway::message::make(slipway::message::naught), replica;
    boost::asio::streambuf buffer;

    BOOST_CHECK(!initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::naught);
    BOOST_CHECK_EQUAL(initial.payload.index(), 0);

    BOOST_REQUIRE_NO_THROW(slipway::push_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::pull_message(buffer, replica));
    BOOST_CHECK(!replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());

    initial = slipway::message::make(slipway::message::adjust, "error");

    BOOST_CHECK(!initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::adjust);
    BOOST_CHECK_EQUAL(initial.payload.index(), 0);
    BOOST_CHECK_EQUAL(std::get<std::string>(initial.payload), "error");

    BOOST_REQUIRE_NO_THROW(slipway::push_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::pull_message(buffer, replica));
    BOOST_CHECK(!replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK_EQUAL(std::get<std::string>(replica.payload), std::get<std::string>(initial.payload));

    slipway::handle ident { 
        "someone@mail.box/pier",
        "service" 
        };
    initial = slipway::message::make(slipway::message::unplug, ident);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::unplug);
    BOOST_CHECK_EQUAL(initial.payload.index(), 1);
    BOOST_CHECK(std::get<slipway::handle>(initial.payload) == ident);

    BOOST_REQUIRE_NO_THROW(slipway::push_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::pull_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<slipway::handle>(replica.payload) == std::get<slipway::handle>(initial.payload));

    slipway::health state { 
        ident,
        slipway::health::active
        };
    initial = slipway::message::make(slipway::message::status, state);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::status);
    BOOST_CHECK_EQUAL(initial.payload.index(), 2);
    BOOST_CHECK(std::get<slipway::health>(initial.payload) == state);

    BOOST_REQUIRE_NO_THROW(slipway::push_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::pull_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<slipway::health>(replica.payload) == std::get<slipway::health>(initial.payload));

    slipway::report report {
        state,
        {
            { "someone@mail.box/pier", 1 },
            { "someoneelse@mail.box/pier", 2 }
        }
    };
    initial = slipway::message::make(slipway::message::review, report);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::review);
    BOOST_CHECK_EQUAL(initial.payload.index(), 3);
    BOOST_CHECK(std::get<slipway::report>(initial.payload) == report);

    BOOST_REQUIRE_NO_THROW(slipway::push_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::pull_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<slipway::report>(replica.payload) == std::get<slipway::report>(initial.payload));

    std::vector<slipway::health> empty;
    initial = slipway::message::make(slipway::message::status, empty);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::status);
    BOOST_CHECK_EQUAL(initial.payload.index(), 4);
    BOOST_CHECK(std::get<std::vector<slipway::health>>(initial.payload) == empty);

    BOOST_REQUIRE_NO_THROW(slipway::push_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::pull_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<std::vector<slipway::health>>(replica.payload) == std::get<std::vector<slipway::health>>(initial.payload));

    std::vector<slipway::health> states { state, state };
    initial = slipway::message::make(slipway::message::status, states);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::status);
    BOOST_CHECK_EQUAL(initial.payload.index(), 4);
    BOOST_CHECK(std::get<std::vector<slipway::health>>(initial.payload) == states);

    BOOST_REQUIRE_NO_THROW(slipway::push_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::pull_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<std::vector<slipway::health>>(replica.payload) == std::get<std::vector<slipway::health>>(initial.payload));

    std::vector<slipway::report> reports { report, report };
    initial = slipway::message::make(slipway::message::review, reports);

    BOOST_CHECK(initial.ok());
    BOOST_CHECK_EQUAL(initial.action, slipway::message::review);
    BOOST_CHECK_EQUAL(initial.payload.index(), 5);
    BOOST_CHECK(std::get<std::vector<slipway::report>>(initial.payload) == reports);

    BOOST_REQUIRE_NO_THROW(slipway::push_message(buffer, initial));
    BOOST_REQUIRE_NO_THROW(slipway::pull_message(buffer, replica));
    BOOST_CHECK(replica.ok());
    BOOST_CHECK_EQUAL(replica.action, initial.action);
    BOOST_CHECK_EQUAL(replica.payload.index(), initial.payload.index());
    BOOST_CHECK(std::get<std::vector<slipway::report>>(replica.payload) == std::get<std::vector<slipway::report>>(initial.payload));
}
