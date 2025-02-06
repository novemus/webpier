#define BOOST_TEST_MODULE webpier_tests

#include <store/utils.h>
#include <boost/test/unit_test.hpp>
#include <boost/scope_exit.hpp>
#include <filesystem>
#include <fstream>

BOOST_AUTO_TEST_CASE(x509)
{
    auto dest = std::filesystem::current_path() / std::to_string(std::time(0));

    BOOST_SCOPE_EXIT(&dest) 
    {
        std::filesystem::remove_all(dest);
    } 
    BOOST_SCOPE_EXIT_END

    BOOST_REQUIRE(std::filesystem::create_directory(dest));

    auto cert = dest / "cert.crt";
    auto copy = dest / "copy.crt";
    auto key = dest / "private.key";

    BOOST_REQUIRE_NO_THROW(webpier::generate_x509_pair(cert, key, "test"));
    BOOST_REQUIRE(std::filesystem::exists(cert));
    BOOST_REQUIRE(std::filesystem::exists(key));

    std::ifstream file(cert.c_str());
    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    BOOST_REQUIRE_NO_THROW(webpier::save_x509_cert(copy, data));
    BOOST_REQUIRE_NO_THROW(BOOST_CHECK_EQUAL(webpier::get_x509_public_sha1(cert), webpier::get_x509_public_sha1(copy)));
    BOOST_REQUIRE_NO_THROW(BOOST_CHECK_EQUAL(webpier::load_x509_cert(cert), webpier::load_x509_cert(copy)));
}
