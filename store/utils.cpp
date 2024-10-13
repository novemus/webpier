#include "utils.h"
#include <memory>
#include <openssl/x509v3.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>

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

    void generate_x509_pair(const std::filesystem::path& cert_path, const std::filesystem::path& key_path, const std::string& subject_name) noexcept(false)
    {
        static constexpr long PERIOD = 10 * 365 * 24 * 3600;

        std::unique_ptr<BIO, void (*)(BIO*)> cert_file(BIO_new_file(cert_path.string().data(), "wb"), BIO_free_all);
        if (!cert_file)
            throw x509_error(get_openssl_error());

        std::unique_ptr<BIO, void (*)(BIO*)> key_file(BIO_new_file(key_path.string().data(), "wb"), BIO_free_all);
        if (!key_file)
            throw x509_error(get_openssl_error());

        std::unique_ptr<EVP_PKEY, decltype(&::EVP_PKEY_free)> pkey(EVP_RSA_gen(2048), EVP_PKEY_free);
        if (!pkey)
            throw x509_error(get_openssl_error());

        std::unique_ptr<X509, void (*)(X509*)> cert(X509_new(), X509_free);
        if (!cert)
            throw x509_error(get_openssl_error());

        ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), 1);

        X509_gmtime_adj(X509_get_notBefore(cert.get()), 0);
        X509_gmtime_adj(X509_get_notAfter(cert.get()), PERIOD);

        X509_set_pubkey(cert.get(), pkey.get());

        X509_name_st* name = X509_get_subject_name(cert.get());
        X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char*>(subject_name.data()), -1, -1, 0);
        X509_set_issuer_name(cert.get(), name);

        if (!X509_sign(cert.get(), pkey.get(), EVP_sha256()))
            throw x509_error(get_openssl_error());

        if (!PEM_write_bio_X509(cert_file.get(), cert.get()))
            throw x509_error(get_openssl_error());

        if (!PEM_write_bio_PrivateKey(key_file.get(), pkey.get(), nullptr, nullptr, 0, nullptr, nullptr))
            throw x509_error(get_openssl_error());
    }

    std::string get_x509_public_hash(const std::filesystem::path& cert_path) noexcept(false)
    {
        unsigned char pubkey_hash[SHA_DIGEST_LENGTH] = {0};

        std::unique_ptr<BIO, void (*)(BIO*)> cert_bio(BIO_new_file(cert_path.string().data(), "r"), BIO_free_all);
        if (!cert_bio)
            throw x509_error(get_openssl_error());

        std::unique_ptr<X509, void (*)(X509*)> cert(PEM_read_bio_X509(cert_bio.get(), NULL, 0, NULL), X509_free);
        if (!cert)
            throw x509_error(get_openssl_error());

        OpenSSL_add_all_digests();

        unsigned int len;
        if (!X509_pubkey_digest(cert.get(), EVP_sha1(), pubkey_hash, &len))
            throw x509_error(get_openssl_error());

        std::stringstream out;
        for (size_t i = 0; i < len; ++i)
            out << std::setw(2) << std::setfill('0') << std::hex << (int)pubkey_hash[i];

        return out.str();
    }

    void save_x509_cert(const std::filesystem::path& cert_path, const std::string& data) noexcept(false)
    {
        std::unique_ptr<BIO, void (*)(BIO*)> cert_bio(BIO_new_mem_buf(data.c_str(), (int)data.size()), BIO_free_all);
        if (!cert_bio)
            throw x509_error(get_openssl_error());

        std::unique_ptr<X509, void (*)(X509*)> cert(PEM_read_bio_X509(cert_bio.get(), NULL, 0, NULL), X509_free);
        if (!cert)
            throw x509_error(get_openssl_error());

        OpenSSL_add_all_digests();

        if(!X509_verify(cert.get(), X509_get_pubkey(cert.get())))
            throw x509_error(get_openssl_error());

        std::unique_ptr<BIO, decltype(&::BIO_free)> file_bio(BIO_new(BIO_s_file()), ::BIO_free);
        if (!file_bio)
            throw x509_error(get_openssl_error());

        if(!BIO_write_filename(file_bio.get(), const_cast<char*>(cert_path.string().data())))
            throw x509_error(get_openssl_error());

        if(!PEM_write_bio_X509(file_bio.get(), cert.get()))
            throw x509_error(get_openssl_error());
    }
}
