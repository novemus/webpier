#include "../context.h"
#include <boost/test/unit_test.hpp>
#include <boost/scope_exit.hpp>
#include <filesystem>

BOOST_AUTO_TEST_CASE(context)
{
    auto dest = std::filesystem::current_path() / std::to_string(std::time(0));

    BOOST_SCOPE_EXIT(&dest) 
    {
        std::filesystem::remove_all(dest);
    } 
    BOOST_SCOPE_EXIT_END

    BOOST_REQUIRE(std::filesystem::create_directory(dest));

    auto context = webpier::open_context(dest, true);
    
    BOOST_REQUIRE(std::filesystem::exists(dest / "webpier.json"));
    BOOST_REQUIRE(std::filesystem::exists(dest / "internal.json"));
    BOOST_REQUIRE(std::filesystem::exists(dest / "external.json"));

    webpier::basic basic {
        "someone@mail.box/test",
        webpier::debug,
        true,
        false,
        {},
        {},
        {
            "smtp.some.com",
            "imap.some.com",
            "someone@mail.box",
            "qwerty",
            "/some/path/cert.pem",
            "/some/path/private.pem",
            "/some/path/ca.pem"
        }};

    BOOST_REQUIRE_NO_THROW(context->set_basic(basic));

    webpier::basic config;
    context->get_basic(config);

    BOOST_CHECK_EQUAL(config.host, basic.host);
    BOOST_CHECK_EQUAL(config.report, basic.report);
    BOOST_CHECK_EQUAL(config.daemon, basic.daemon);
    BOOST_CHECK_EQUAL(config.tray, basic.tray);
    BOOST_CHECK_EQUAL(config.traverse.stun, basic.traverse.stun);
    BOOST_CHECK_EQUAL(config.traverse.hops, basic.traverse.hops);
    BOOST_CHECK_EQUAL(config.rendezvous.bootstrap, basic.rendezvous.bootstrap);
    BOOST_CHECK_EQUAL(config.rendezvous.network, basic.rendezvous.network);
    BOOST_CHECK_EQUAL(config.emailer.smtp, basic.emailer.smtp);
    BOOST_CHECK_EQUAL(config.emailer.imap, basic.emailer.imap);
    BOOST_CHECK_EQUAL(config.emailer.login, basic.emailer.login);
    BOOST_CHECK_EQUAL(config.emailer.password, basic.emailer.password);
    BOOST_CHECK_EQUAL(config.emailer.password, basic.emailer.password);
    BOOST_CHECK_EQUAL(config.emailer.password, basic.emailer.password);
    BOOST_CHECK_EQUAL(config.emailer.password, basic.emailer.password);

    webpier::service service {
        "foo"
        "someone@mail.box/test",
        "127.0.0.1:1234",
        "127.0.0.1:5678",
        "someone@mail.box",
        {},
        true,
        false
        };

    BOOST_REQUIRE_NO_THROW(context->add_local_service(service));

    std::vector<webpier::service> locals;
    context->get_local_services(locals);

    BOOST_REQUIRE_EQUAL(locals.size(), 1);
    BOOST_CHECK_EQUAL(service.id, locals[0].id);
    BOOST_CHECK_EQUAL(service.peer, locals[0].peer);
    BOOST_CHECK_EQUAL(service.service, locals[0].service);
    BOOST_CHECK_EQUAL(service.gateway, locals[0].gateway);
    BOOST_CHECK_EQUAL(service.rendezvous.bootstrap, locals[0].rendezvous.bootstrap);
    BOOST_CHECK_EQUAL(service.rendezvous.network, locals[0].rendezvous.network);
    BOOST_CHECK_EQUAL(service.autostart, locals[0].autostart);
    BOOST_CHECK_EQUAL(service.obscure, locals[0].obscure);

    BOOST_REQUIRE_NO_THROW(context->add_remote_service(service));

    std::vector<webpier::service> remotes;
    context->get_remote_services(remotes);

    BOOST_REQUIRE_EQUAL(remotes.size(), 1);
    BOOST_CHECK_EQUAL(service.id, remotes[0].id);
    BOOST_CHECK_EQUAL(service.peer, remotes[0].peer);
    BOOST_CHECK_EQUAL(service.service, remotes[0].service);
    BOOST_CHECK_EQUAL(service.gateway, remotes[0].gateway);
    BOOST_CHECK_EQUAL(service.rendezvous.bootstrap, remotes[0].rendezvous.bootstrap);
    BOOST_CHECK_EQUAL(service.rendezvous.network, remotes[0].rendezvous.network);
    BOOST_CHECK_EQUAL(service.autostart, remotes[0].autostart);
    BOOST_CHECK_EQUAL(service.obscure, remotes[0].obscure);

    BOOST_REQUIRE_NO_THROW(context->del_local_service(service.id));
    BOOST_REQUIRE_NO_THROW(context->del_remote_service(service.peer, service.id));

    locals.clear();
    context->get_local_services(locals);
    BOOST_REQUIRE(locals.empty());

    remotes.clear();
    context->get_remote_services(remotes);
    BOOST_REQUIRE(remotes.empty());
}