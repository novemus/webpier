#pragma once

#include <cstdio>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/udp.hpp>

namespace webpier
{
    constexpr const char* webpier_module = "webpier";
    constexpr const char* slipway_module = "slipway";
    constexpr const char* carrier_module = "carrier";

#ifdef WIN32
    constexpr const char* taskxml_config = "taskxml";
#endif

    constexpr const char* stun_server_default_port = "3478";
    constexpr const char* stun_client_default_port = "0";
    constexpr const char* smtp_server_default_port = "smtps";
    constexpr const char* imap_server_default_port = "imaps";

    struct x509_error : public std::runtime_error { x509_error(const std::string& what) : std::runtime_error(what) {} };

    void generate_x509_pair(const std::filesystem::path& cert_path, const std::filesystem::path& key_path, const std::string& subject_name) noexcept(false);
    void save_x509_cert(const std::filesystem::path& cert_path, const std::string& data) noexcept(false);
    std::string load_x509_cert(const std::filesystem::path& cert_path) noexcept(false);
    std::string get_x509_public_sha1(const std::filesystem::path& cert_path) noexcept(false);
    std::string to_hexadecimal(const void* data, size_t len) noexcept(true);
    std::string make_timestamp(const char* format) noexcept(true);
    std::string hexify(uint64_t value) noexcept(true);
    std::string locale_to_utf8(const std::string& str) noexcept(false);
    std::string utf8_to_locale(const std::string& str) noexcept(false);
    std::filesystem::path get_module_path(const std::string& module) noexcept(false);
    std::filesystem::path get_absolute_path(const std::string& file) noexcept(false);
    bool verify_autostart(const std::filesystem::path& exec, const std::string& args) noexcept(false);
    void assign_autostart(const std::filesystem::path& exec, const std::string& args) noexcept(false);
    void revoke_autostart(const std::filesystem::path& exec, const std::string& args) noexcept(false);
    boost::asio::ip::udp::endpoint make_udp_endpoint(const std::string& url, const std::string& service) noexcept(false);
    boost::asio::ip::tcp::endpoint make_tcp_endpoint(const std::string& url, const std::string& service) noexcept(false);
}
