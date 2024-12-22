#include "../context.h"
#include "../utils.h"
#include <boost/test/unit_test.hpp>
#include <boost/scope_exit.hpp>
#include <filesystem>

BOOST_AUTO_TEST_CASE(context)
{
    auto dest = std::filesystem::current_path() / std::to_string(std::time(0));
    auto link = dest / "context";

    BOOST_REQUIRE_NO_THROW(std::filesystem::create_directory(dest));

    BOOST_SCOPE_EXIT(&dest) 
    {
        std::filesystem::remove_all(dest);
    } 
    BOOST_SCOPE_EXIT_END

    std::string host = "host@mail.box/test";
    std::string peer = "peer@mail.box/test";

    auto context = webpier::open_context(dest);

    webpier::config in {
        peer,
        {},
        {},
        {
            "smtp.some.com",
            "imap.some.com",
            "host@mail.box",
            "qwerty",
            "/some/path/cert.pem",
            "/some/path/private.pem",
            "/some/path/ca.pem"
        }};

    BOOST_REQUIRE_NO_THROW(context->set_config(in));

    BOOST_REQUIRE(std::filesystem::is_symlink(link));
    BOOST_REQUIRE(std::filesystem::is_directory(std::filesystem::read_symlink(link)));

    in.host = host;

    BOOST_REQUIRE_NO_THROW(context->set_config(in));

    webpier::config out;
    context->get_config(out);

    BOOST_CHECK_EQUAL(out.host, in.host);
    BOOST_CHECK_EQUAL(out.traverse.stun, in.traverse.stun);
    BOOST_CHECK_EQUAL(out.traverse.hops, in.traverse.hops);
    BOOST_CHECK_EQUAL(out.rendezvous.bootstrap, in.rendezvous.bootstrap);
    BOOST_CHECK_EQUAL(out.rendezvous.network, in.rendezvous.network);
    BOOST_CHECK_EQUAL(out.emailer.smtp, in.emailer.smtp);
    BOOST_CHECK_EQUAL(out.emailer.imap, in.emailer.imap);
    BOOST_CHECK_EQUAL(out.emailer.login, in.emailer.login);
    BOOST_CHECK_EQUAL(out.emailer.password, in.emailer.password);
    BOOST_CHECK_EQUAL(out.emailer.cert, in.emailer.cert);
    BOOST_CHECK_EQUAL(out.emailer.key, in.emailer.key);
    BOOST_CHECK_EQUAL(out.emailer.ca, in.emailer.ca);
    BOOST_CHECK_EQUAL(out.autostart, in.autostart);

    webpier::service service {
        "foo",
        peer,
        "127.0.0.1:1234",
        "127.0.0.1:5678",
        {},
        true,
        false
        };

    webpier::generate_x509_pair(dest / "cert.crt", dest / "private.key", peer);
    auto peer_certificate = webpier::load_x509_cert(dest / "cert.crt");
    auto peer_fingerprint = webpier::get_x509_public_sha1(dest / "cert.crt");

    BOOST_REQUIRE_NO_THROW(context->add_peer(peer, peer_certificate));
    BOOST_REQUIRE_THROW(context->add_peer(peer, peer_certificate), webpier::usage_error);

    std::vector<std::string> list;
    context->get_peers(list);

    BOOST_REQUIRE_EQUAL(list.size(), 1);
    BOOST_REQUIRE_EQUAL(list[0], peer);

    BOOST_REQUIRE_NO_THROW(BOOST_CHECK_EQUAL(context->get_certificate(peer), peer_certificate));
    BOOST_REQUIRE_NO_THROW(BOOST_CHECK_EQUAL(context->get_fingerprint(peer), peer_fingerprint));

    BOOST_REQUIRE_NO_THROW(context->add_export_service(service));
    BOOST_REQUIRE_THROW(context->add_export_service(service), webpier::usage_error);

    BOOST_REQUIRE_NO_THROW(context->add_import_service(service));
    BOOST_REQUIRE_THROW(context->add_import_service(service), webpier::usage_error);

    service.name = "bar";

    BOOST_REQUIRE_NO_THROW(context->add_export_service(service));
    BOOST_REQUIRE_THROW(context->add_export_service(service), webpier::usage_error);
    
    BOOST_REQUIRE_NO_THROW(context->add_import_service(service));
    BOOST_REQUIRE_THROW(context->add_import_service(service), webpier::usage_error);

    BOOST_REQUIRE_NO_THROW(context->del_export_service("foo"));
    BOOST_REQUIRE_NO_THROW(context->del_import_service(service.peer, "foo"));

    std::vector<webpier::service> locals;
    context->get_export_services(locals);

    BOOST_REQUIRE_EQUAL(locals.size(), 1);
    BOOST_CHECK_EQUAL(service.name, locals[0].name);
    BOOST_CHECK_EQUAL(service.peer, locals[0].peer);
    BOOST_CHECK_EQUAL(service.address, locals[0].address);
    BOOST_CHECK_EQUAL(service.gateway, locals[0].gateway);
    BOOST_CHECK_EQUAL(service.rendezvous.bootstrap, locals[0].rendezvous.bootstrap);
    BOOST_CHECK_EQUAL(service.rendezvous.network, locals[0].rendezvous.network);
    BOOST_CHECK_EQUAL(service.autostart, locals[0].autostart);
    BOOST_CHECK_EQUAL(service.obscure, locals[0].obscure);

    std::vector<webpier::service> remotes;
    context->get_import_services(remotes);

    BOOST_REQUIRE_EQUAL(remotes.size(), 1);
    BOOST_CHECK_EQUAL(service.name, remotes[0].name);
    BOOST_CHECK_EQUAL(service.peer, remotes[0].peer);
    BOOST_CHECK_EQUAL(service.address, remotes[0].address);
    BOOST_CHECK_EQUAL(service.gateway, remotes[0].gateway);
    BOOST_CHECK_EQUAL(service.rendezvous.bootstrap, remotes[0].rendezvous.bootstrap);
    BOOST_CHECK_EQUAL(service.rendezvous.network, remotes[0].rendezvous.network);
    BOOST_CHECK_EQUAL(service.autostart, remotes[0].autostart);
    BOOST_CHECK_EQUAL(service.obscure, remotes[0].obscure);

    BOOST_REQUIRE_NO_THROW(context->del_peer(peer));

    remotes.clear();
    context->get_import_services(remotes);
    BOOST_REQUIRE(remotes.empty());
}
