#include <store/utils.h>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <cwchar>
#include <codecvt>
#include <openssl/x509v3.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <boost/process.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef WIN32
    #include <windows.h>
    #include <boost/process/windows.hpp>
#elif __APPLE__
    #include <sysdir.h>
#endif

namespace webpier
{
    std::string get_openssl_error()
    {
        std::string ssl = ERR_error_string(ERR_get_error(), NULL);
        std::string sys = strerror(errno);
        if (ssl.empty())
            return sys;
        if (sys.empty())
            return ssl;
        return ssl + "\n" + sys;
    };

    void calc_digest(EVP_PKEY* key, const EVP_MD *type, unsigned char *hash, unsigned int *size)
    {
        std::vector<unsigned char> buf;
        buf.resize(i2d_PUBKEY(key, NULL));
        unsigned char* p = buf.data();
        i2d_PUBKEY(key, &p);

        if (!EVP_Digest(buf.data(), buf.size(), hash, size, EVP_sha1(), NULL))
            throw x509_error(get_openssl_error());
    }

    void generate_x509_pair(const std::filesystem::path& cert_path, const std::filesystem::path& key_path, const std::string& subject_name) noexcept(false)
    {
        static constexpr long PERIOD = 10 * 365 * 24 * 3600;

        std::unique_ptr<BIO, void (*)(BIO*)> cert_file(BIO_new_file(cert_path.string().data(), "wb"), BIO_free_all);
        if (!cert_file)
            throw x509_error(get_openssl_error());

        std::unique_ptr<BIO, void (*)(BIO*)> key_file(BIO_new_file(key_path.string().data(), "wb"), BIO_free_all);
        if (!key_file)
            throw x509_error(get_openssl_error());

        std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY*)> pkey(EVP_RSA_gen(2048), EVP_PKEY_free);
        if (!pkey)
            throw x509_error(get_openssl_error());

        std::unique_ptr<X509, void (*)(X509*)> cert(X509_new(), X509_free);
        if (!cert)
            throw x509_error(get_openssl_error());

        X509_set_version(cert.get(), X509_VERSION_3);

        unsigned char hash[SHA_LBLOCK];
        RAND_bytes(hash, SHA_LBLOCK);
        ASN1_OCTET_STRING_set(X509_get_serialNumber(cert.get()), hash, SHA_LBLOCK);

        X509_gmtime_adj(X509_get_notBefore(cert.get()), 0);
        X509_gmtime_adj(X509_get_notAfter(cert.get()), PERIOD);

        X509_set_pubkey(cert.get(), pkey.get());

        X509_name_st* name = X509_get_subject_name(cert.get());
        X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char*>(subject_name.data()), -1, -1, 0);
        X509_set_issuer_name(cert.get(), name);

        X509V3_CTX ctx;
        X509V3_set_ctx_nodb(&ctx);
        X509V3_set_ctx(&ctx, nullptr, cert.get(), nullptr, nullptr, 0);

        std::unique_ptr<X509_EXTENSION, void (*)(X509_EXTENSION*)> ext1(X509V3_EXT_conf_nid(nullptr, &ctx, NID_basic_constraints, "CA:FALSE"), X509_EXTENSION_free);
        if (!ext1)
            throw x509_error(get_openssl_error());
        
        X509_add_ext(cert.get(), ext1.get(), -1);

        std::unique_ptr<X509_EXTENSION, void (*)(X509_EXTENSION*)> ext2(X509V3_EXT_conf_nid(nullptr, &ctx, NID_key_usage, "digitalSignature"), X509_EXTENSION_free);
        if (!ext2)
            throw x509_error(get_openssl_error());

        X509_add_ext(cert.get(), ext2.get(), -1);

        unsigned char digest[SHA_DIGEST_LENGTH];
        calc_digest(pkey.get(), EVP_sha1(), digest, nullptr);
        std::unique_ptr<char, void (*)(char*)> value(OPENSSL_buf2hexstr(digest, SHA_DIGEST_LENGTH), [](char* ptr) { OPENSSL_free(ptr); });

        std::unique_ptr<X509_EXTENSION, void (*)(X509_EXTENSION*)> ext3(X509V3_EXT_conf_nid(nullptr, &ctx, NID_subject_key_identifier, value.get()), X509_EXTENSION_free);
        if (!ext3)
            throw x509_error(get_openssl_error());

        X509_add_ext(cert.get(), ext3.get(), -1);

        if (!X509_sign(cert.get(), pkey.get(), EVP_sha256()))
            throw x509_error(get_openssl_error());

        if (!PEM_write_bio_X509(cert_file.get(), cert.get()))
            throw x509_error(get_openssl_error());

        if (!PEM_write_bio_PrivateKey(key_file.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr))
            throw x509_error(get_openssl_error());
    }

    std::string get_x509_public_sha1(const std::filesystem::path& cert_path) noexcept(false)
    {
        std::unique_ptr<BIO, void (*)(BIO*)> cert_bio(BIO_new_file(cert_path.string().data(), "r"), BIO_free_all);
        if (!cert_bio)
            throw x509_error(get_openssl_error());

        std::unique_ptr<X509, void (*)(X509*)> cert(PEM_read_bio_X509(cert_bio.get(), NULL, 0, NULL), X509_free);
        if (!cert)
            throw x509_error(get_openssl_error());

        unsigned char hash[SHA_DIGEST_LENGTH];
        calc_digest(X509_get_pubkey(cert.get()), EVP_sha1(), hash, nullptr);

        std::stringstream out;
        for (size_t i = 0; i < SHA_DIGEST_LENGTH; ++i)
            out << std::setw(2) << std::setfill('0') << std::hex << (int)hash[i];

        return out.str();
    }

    std::string load_x509_cert(const std::filesystem::path& cert_path) noexcept(false)
    {
        std::ifstream file(cert_path.string());
        std::string buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        std::unique_ptr<BIO, void (*)(BIO*)> cert_bio(BIO_new_mem_buf(buffer.data(), (int)buffer.size()), BIO_free_all);
        if (!cert_bio)
            throw x509_error(get_openssl_error());

        std::unique_ptr<X509, void (*)(X509*)> cert(PEM_read_bio_X509(cert_bio.get(), NULL, 0, NULL), X509_free);
        if (!cert)
            throw x509_error(get_openssl_error());

        if(!X509_verify(cert.get(), X509_get_pubkey(cert.get())))
            throw x509_error(get_openssl_error());

        return buffer;
    }

    void save_x509_cert(const std::filesystem::path& cert_path, const std::string& data) noexcept(false)
    {
        std::unique_ptr<BIO, void (*)(BIO*)> cert_bio(BIO_new_mem_buf(data.c_str(), (int)data.size()), BIO_free_all);
        if (!cert_bio)
            throw x509_error(get_openssl_error());

        std::unique_ptr<X509, void (*)(X509*)> cert(PEM_read_bio_X509(cert_bio.get(), NULL, 0, NULL), X509_free);
        if (!cert)
            throw x509_error(get_openssl_error());

        if(!X509_verify(cert.get(), X509_get_pubkey(cert.get())))
            throw x509_error(get_openssl_error());

        std::unique_ptr<BIO, void (*)(BIO*)> file_bio(BIO_new(BIO_s_file()), BIO_free_all);
        if (!file_bio)
            throw x509_error(get_openssl_error());

        if(!BIO_write_filename(file_bio.get(), const_cast<char*>(cert_path.string().data())))
            throw x509_error(get_openssl_error());

        if(!PEM_write_bio_X509(file_bio.get(), cert.get()))
            throw x509_error(get_openssl_error());
    }

    std::string to_hexadecimal(const void* data, size_t len) noexcept(true)
    {
        std::stringstream out;
        for (size_t i = 0; i < len; ++i)
        {
            out << std::setw(2) << std::setfill('0') << std::hex << (int)((uint8_t*)data)[i];
        }
        return out.str();
    }

    std::string locale_to_utf8(const std::string& str) noexcept(false)
    {
        if (str.empty())
            return str;
#ifdef WIN32
        int len = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), 0, 0);
        std::wstring temp(len, L'\0');
        ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), temp.data(), (int)temp.length());
        
        len = ::WideCharToMultiByte(CP_UTF8, 0, temp.c_str(), (int)temp.length(), NULL, 0, NULL, NULL);
        std::string res(len, 0);
        ::WideCharToMultiByte(CP_UTF8, 0, temp.c_str(), (int)temp.length(), res.data(), (int)res.length(), NULL, NULL);

        return res;
#else
        return str;
#endif
    }

    std::string utf8_to_locale(const std::string& str) noexcept(false)
    {
        if (str.empty())
            return str;
#ifdef WIN32
        int len = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), 0, 0);
        std::wstring temp(len, L'\0');
        ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), temp.data(), (int)temp.length());

        len = ::WideCharToMultiByte(CP_ACP, 0, temp.c_str(), (int)temp.length(), NULL, 0, NULL, NULL);
        std::string res(len, 0);
        ::WideCharToMultiByte(CP_ACP, 0, temp.c_str(), (int)temp.length(), res.data(), (int)res.length(), NULL, NULL);

        return res;
#else
        return str;
#endif
    }

    std::string hexify(uint64_t value) noexcept(true)
    {
        std::stringstream ss;
        ss << "0x" << std::setfill('0') << std::setw(sizeof(uint64_t) * 2) << std::hex << value;
        return ss.str();
    }

    std::string make_timestamp(const char* format) noexcept(true)
    {
        std::time_t time = std::time(0);
        std::tm tm = *std::localtime(&time);
        std::stringstream ss;
        ss << std::put_time(&tm, format);
        return ss.str();
    }

    std::filesystem::path search_file(const std::filesystem::path& dir, const std::filesystem::path& name)
    {
        for (const auto& item : std::filesystem::directory_iterator(dir))
        {
            if (item.is_directory())
            {
                auto path = search_file(item.path(), name);
                if (!path.empty())
                    return path;
            }

            if (item.path().filename() == name)
                return item.path();
        }

        return std::filesystem::path();
    }

    std::filesystem::path get_module_path(const std::string& module) noexcept(false)
    {
#ifndef WEBPIER_CONFIG
        static const std::map<std::string, std::string> s_config {
    #ifdef WIN32
            { taskxml_config, TASKXML_FILE_NAME },
    #endif
            { webpier_module, WEBPIER_FILE_NAME },
            { slipway_module, SLIPWAY_FILE_NAME },
            { carrier_module, CARRIER_FILE_NAME }
        };

        auto iter = s_config.find(module);
        if (iter == s_config.end())
            throw std::runtime_error("Unknown module \"" + module + "\" name");

        auto file = search_file(std::filesystem::current_path(), iter->second);
        if (file.empty())
            throw std::runtime_error("Can't find module \"" + module + "\" path");

        return file;
#else
    #ifdef WIN32
        std::string value;
        value.resize(0xFFF);

        DWORD size = 0;
        auto rc = RegGetValueA(HKEY_LOCAL_MACHINE, WEBPIER_CONFIG, module.c_str(), RRF_RT_REG_SZ, nullptr, static_cast<void*>(value.data()), &size);

        if (rc == ERROR_MORE_DATA)
        {
            value.resize(size / sizeof(char));
            rc = RegGetValueA(HKEY_LOCAL_MACHINE, WEBPIER_CONFIG, module.c_str(), RRF_RT_REG_SZ, nullptr, static_cast<void*>(value.data()), &size);
        }

        if (rc != ERROR_SUCCESS)
            throw std::runtime_error("Can't get module \"" + module + "\" path: error=" + webpier::hexify(rc));

        value.resize(size / sizeof(char) - 1);
        return std::filesystem::path(value);
    #else
        static std::once_flag s_flag;
        static boost::program_options::variables_map s_config;

        std::call_once(s_flag, [&]()
        {
            std::filesystem::path path(WEBPIER_CONFIG);
            if (!std::filesystem::exists(path))
                throw std::runtime_error("No configuration file");

            boost::program_options::options_description options;
            options.add_options()
                (webpier_module, boost::program_options::value<std::string>())
                (slipway_module, boost::program_options::value<std::string>())
                (carrier_module, boost::program_options::value<std::string>());

            boost::program_options::store(boost::program_options::parse_config_file<char>(path.string().c_str(), options, true), s_config);
        });

        if (!s_config.count(module))
            throw std::runtime_error("Unknown module \"" + module + "\" name");

        return s_config[module].as<std::string>();
    #endif
#endif
    }

    bool verify_autostart(const std::filesystem::path& exec, const std::string& args) noexcept(false)
    {
#ifndef WIN32
        std::string record = "@reboot " + exec.string() + " " + args;

        boost::process::ipstream is;
        boost::process::child read("crontab -l", boost::process::std_out > is);
        std::string line;

        bool seen = false;
        while (read.running() && std::getline(is, line))
        {
            if (line == record)
                seen = true;
        }

        read.wait();
        return seen;
#else
        std::string id = std::to_string(std::hash<std::string>()(exec.string() + args));
        boost::process::child proc("schtasks /Query /TN \"\\WebPier\\Task #" + id + "\" /HRESULT", boost::process::windows::hide);
        proc.wait();

        return proc.exit_code() == ERROR_SUCCESS;
#endif
    }

    void assign_autostart(const std::filesystem::path& exec, const std::string& args) noexcept(false)
    {
#ifndef WIN32
        std::string record = "@reboot " + exec.string() + " " + args;

        boost::process::ipstream is;
        boost::process::opstream os;

        boost::process::child read("crontab -l", boost::process::std_out > is);
        std::string line;

        bool seen = false;
        while (read.running() && std::getline(is, line))
        {
            os << line << std::endl;

            if (line == record)
                seen = true;
        }

        read.wait();

        if (!seen)
        {
            os << record << std::endl;

            boost::process::child write("crontab", boost::process::std_in < os);
            os.pipe().close();
            write.wait();
        }
#else
        boost::property_tree::ptree taskxml;
        boost::property_tree::read_xml(get_module_path(webpier::taskxml_config).string(), taskxml);
        taskxml.put("Task.RegistrationInfo.Date", webpier::make_timestamp("%Y-%m-%dT%H:%M:%S"));
        taskxml.put("Task.Actions.Exec.Command", exec.string());
        taskxml.put("Task.Actions.Exec.Arguments", args);

        std::string id = std::to_string(std::hash<std::string>()(exec.string() + args));
        std::filesystem::path xmlpath = std::filesystem::temp_directory_path() / (id + ".xml");

        std::locale locale(std::locale::classic(), new std::codecvt_utf16<wchar_t, 0x10ffff, (std::codecvt_mode)(std::generate_header | std::little_endian)>);
        boost::property_tree::xml_writer_settings<std::string> settings('\t', 1, "UTF-16");
        boost::property_tree::write_xml(xmlpath.string(), taskxml, locale, settings);

        boost::process::child proc("schtasks /Create /TN \"\\WebPier\\Task #" + id + "\" /XML \"" + xmlpath.string() + "\" /F /HRESULT", boost::process::windows::hide);
        proc.wait();

        if (proc.exit_code() != ERROR_SUCCESS)
            throw std::runtime_error("Can't create the task: error=" + webpier::hexify(proc.exit_code()));
#endif
    }

    void revoke_autostart(const std::filesystem::path& exec, const std::string& args) noexcept(false)
    {
#ifndef WIN32
        std::string record = "@reboot " + exec.string() + " " + args;

        boost::process::ipstream is;
        boost::process::opstream os;

        boost::process::child read("crontab -l", boost::process::std_out > is);
        std::string line;

        bool seen = false;
        while (read.running() && std::getline(is, line))
        {
            if (line != record)
                os << line << std::endl;
            else
                seen = true;
        }

        read.wait();

        if (seen)
        {
            boost::process::child write("crontab", boost::process::std_in < os);
            os.pipe().close();
            write.wait();
        }
#else
        std::string id = std::to_string(std::hash<std::string>()(exec.string() + args));
        boost::process::child proc("schtasks /Delete /TN \"\\WebPier\\Task #" + id + "\" /F /HRESULT", boost::process::windows::hide);
        proc.wait();

        if (proc.exit_code() != ERROR_SUCCESS)
            throw std::runtime_error("Can't revoke the task: error=" + webpier::hexify(proc.exit_code()));
#endif
    }
}
