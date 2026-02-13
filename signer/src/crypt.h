//
// Created by kapil on 13.02.2026.
//

#ifndef JAPI_PRELOAD_CRYPT_H
#define JAPI_PRELOAD_CRYPT_H
#include <memory>
#include <vector>

#include "sodium/crypto_sign.h"

struct keychain {
    uint8_t public_key[crypto_sign_PUBLICKEYBYTES]{};
    uint8_t private_key[crypto_sign_SECRETKEYBYTES]{};
};

class crypt {
public:
    static crypt* get();

    std::string to_base64(const std::vector<uint8_t>& data);
    std::vector<uint8_t> from_base64(const std::string& b64_string);

    keychain load_keys_from_file();
    keychain& get_public_keychain();

    keychain generate_pub_priv_keypair();
    std::vector<uint8_t> create_signature(const std::vector<uint8_t>& data, const keychain& keys);
    bool verify_signature(const std::vector<uint8_t>& data, const std::vector<uint8_t>& signature, const keychain& keys);
private:
    crypt();

    static inline std::unique_ptr<crypt> _instance = nullptr;
};

#endif //JAPI_PRELOAD_CRYPT_H