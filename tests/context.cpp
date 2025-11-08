#include <store/context.h>
#include <store/utils.h>
#include <boost/test/unit_test.hpp>
#include <boost/scope_exit.hpp>
#include <boost/filesystem.hpp>
#include <filesystem>

BOOST_AUTO_TEST_CASE(context)
{
    std::string host = "host@mail.box/test";
    std::string peer = "peer@mail.box/test";

    auto dest = std::filesystem::current_path() / boost::filesystem::unique_path().string();
    auto hrep = dest / webpier::make_text_hash(host);
    auto prep = dest / webpier::make_text_hash(peer);

    BOOST_REQUIRE_NO_THROW(std::filesystem::create_directory(dest));

    BOOST_SCOPE_EXIT(&dest) 
    {
        std::filesystem::remove_all(dest);
    } 
    BOOST_SCOPE_EXIT_END

    auto context = webpier::open_context(dest.string());

    webpier::config in {
        peer,
        prep.string(),
        { dest.string() },
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

    in.pier = host;
    in.repo = hrep.string();

    BOOST_REQUIRE_NO_THROW(context->set_config(in));

    webpier::config out;
    context->get_config(out);

    BOOST_CHECK_EQUAL(out.pier, in.pier);
    BOOST_CHECK_EQUAL(out.nat.stun, in.nat.stun);
    BOOST_CHECK_EQUAL(out.nat.hops, in.nat.hops);
    BOOST_CHECK_EQUAL(out.dht.bootstrap, in.dht.bootstrap);
    BOOST_CHECK_EQUAL(out.dht.port, in.dht.port);
    BOOST_CHECK_EQUAL(out.email.smtp, in.email.smtp);
    BOOST_CHECK_EQUAL(out.email.imap, in.email.imap);
    BOOST_CHECK_EQUAL(out.email.login, in.email.login);
    BOOST_CHECK_EQUAL(out.email.password, in.email.password);
    BOOST_CHECK_EQUAL(out.email.cert, in.email.cert);
    BOOST_CHECK_EQUAL(out.email.key, in.email.key);
    BOOST_CHECK_EQUAL(out.email.ca, in.email.ca);

    webpier::service service {
        true,
        "foo",
        peer,
        "127.0.0.1:1234",
        "127.0.0.1:4321",
        "bootstrap.dht",
        true,
        false
        };

    webpier::generate_x509_pair(dest / "cert.crt", dest / "private.key", peer);
    auto peer_certificate = webpier::load_x509_cert(dest / "cert.crt");
    auto peer_fingerprint = webpier::get_x509_public_sha1(dest / "cert.crt");

    BOOST_REQUIRE_NO_THROW(context->add_pier(peer, peer_certificate));
    BOOST_REQUIRE_THROW(context->add_pier(peer, peer_certificate), webpier::usage_error);

    std::vector<std::string> list;
    context->get_piers(list);

    BOOST_REQUIRE_EQUAL(list.size(), 1);
    BOOST_REQUIRE_EQUAL(list[0], peer);

    BOOST_REQUIRE_NO_THROW(BOOST_CHECK_EQUAL(context->get_certificate(peer), peer_certificate));
    BOOST_REQUIRE_NO_THROW(BOOST_CHECK_EQUAL(context->get_fingerprint(peer), peer_fingerprint));

    BOOST_REQUIRE_NO_THROW(context->add_export_service(service));
    BOOST_REQUIRE_THROW(context->add_export_service(service), webpier::usage_error);

    service.local = false;

    BOOST_REQUIRE_NO_THROW(context->add_import_service(service));
    BOOST_REQUIRE_THROW(context->add_import_service(service), webpier::usage_error);

    service.local = true;
    service.name = "bar";

    BOOST_REQUIRE_NO_THROW(context->add_export_service(service));
    BOOST_REQUIRE_THROW(context->add_export_service(service), webpier::usage_error);
    
    service.local = false;

    BOOST_REQUIRE_NO_THROW(context->add_import_service(service));
    BOOST_REQUIRE_THROW(context->add_import_service(service), webpier::usage_error);

    BOOST_REQUIRE_NO_THROW(context->del_export_service("foo"));
    BOOST_REQUIRE_NO_THROW(context->del_import_service(service.pier, "foo"));

    std::vector<webpier::service> locals;
    context->get_export_services(locals);

    BOOST_REQUIRE_EQUAL(locals.size(), 1);
    BOOST_CHECK_EQUAL(true, locals[0].local);
    BOOST_CHECK_EQUAL(service.name, locals[0].name);
    BOOST_CHECK_EQUAL(service.pier, locals[0].pier);
    BOOST_CHECK_EQUAL(service.address, locals[0].address);
    BOOST_CHECK_EQUAL(service.gateway, locals[0].gateway);
    BOOST_CHECK_EQUAL(service.rendezvous, locals[0].rendezvous);
    BOOST_CHECK_EQUAL(service.autostart, locals[0].autostart);
    BOOST_CHECK_EQUAL(service.obscure, locals[0].obscure);

    std::vector<webpier::service> remotes;
    context->get_import_services(remotes);

    BOOST_REQUIRE_EQUAL(remotes.size(), 1);
    BOOST_CHECK_EQUAL(false, remotes[0].local);
    BOOST_CHECK_EQUAL(service.name, remotes[0].name);
    BOOST_CHECK_EQUAL(service.pier, remotes[0].pier);
    BOOST_CHECK_EQUAL(service.address, remotes[0].address);
    BOOST_CHECK_EQUAL(service.gateway, remotes[0].gateway);
    BOOST_CHECK_EQUAL(service.rendezvous, remotes[0].rendezvous);
    BOOST_CHECK_EQUAL(service.autostart, remotes[0].autostart);
    BOOST_CHECK_EQUAL(service.obscure, remotes[0].obscure);

    BOOST_REQUIRE_NO_THROW(context->del_pier(peer));

    remotes.clear();
    context->get_import_services(remotes);
    BOOST_REQUIRE(remotes.empty());
}
