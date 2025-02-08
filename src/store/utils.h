#pragma once

#include <cstdio>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <boost/filesystem.hpp>

namespace webpier
{
    struct x509_error : public std::runtime_error { x509_error(const std::string& what) : std::runtime_error(what) {} };

    void generate_x509_pair(const std::filesystem::path& cert_path, const std::filesystem::path& key_path, const std::string& subject_name) noexcept(false);
    void save_x509_cert(const std::filesystem::path& cert_path, const std::string& data) noexcept(false);
    std::string load_x509_cert(const std::filesystem::path& cert_path) noexcept(false);
    std::string get_x509_public_sha1(const std::filesystem::path& cert_path) noexcept(false);
    std::string to_hexadecimal(const void* data, size_t len) noexcept(true);
    boost::filesystem::path get_module_path(const std::string& module) noexcept(false);
}
