#pragma once

#include <cstdio>
#include <string>
#include <stdexcept>
#include <filesystem>

namespace webpier
{
    struct x509_error : public std::runtime_error { x509_error(const std::string& what) : std::runtime_error(what) {} };

    void generate_x509_pair(const std::filesystem::path& cert_path, const std::filesystem::path& key_path, const std::string& subject_name) noexcept(false);
    void save_x509_cert(const std::filesystem::path& cert_path, const std::string& data) noexcept(false);
    std::string load_x509_cert(const std::filesystem::path& cert_path) noexcept(false);
    std::string get_x509_public_sha1(const std::filesystem::path& cert_path) noexcept(false);
    std::string to_hexadecimal(const void* data, size_t len) noexcept(true);
    std::filesystem::path get_module_path(const std::string& module) noexcept(false);
    bool verify_autostart(const std::filesystem::path& exec, const std::string& args) noexcept(false);
    void assign_autostart(const std::filesystem::path& exec, const std::string& args) noexcept(false);
    void revoke_autostart(const std::filesystem::path& exec, const std::string& args) noexcept(false);

    template<typename char_t>
    std::basic_string<char_t> make_timestamp(const char_t* format) noexcept(true)
    {
        std::time_t time = std::time(0);
        std::tm tm = *std::localtime(&time);
        std::basic_stringstream<char_t> ss;
        ss << std::put_time(&tm, format);
        return ss.str();
    }

    template< typename integer_t>
    std::string hexify(integer_t value)
    {
        std::stringstream ss;
        ss << "0x" << std::setfill('0') << std::setw(sizeof(integer_t) * 2) << std::hex << value;
        return ss.str();
    }
}
