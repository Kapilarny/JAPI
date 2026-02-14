//
// Created by kapil on 13.02.2026.
//

#include "crypt.h"

#include <sodium.h>
#include <fstream>

#include "public_key.h"

crypt* crypt::get() {
    if (_instance) return _instance.get();
    _instance = std::unique_ptr<crypt>(new crypt());

    return _instance.get();
}

std::string crypt::to_base64(const std::vector<uint8_t> &data) {
    size_t b64_len = sodium_base64_encoded_len(data.size(), sodium_base64_VARIANT_ORIGINAL);
    std::string b64_string(b64_len, '\0');
    sodium_bin2base64(b64_string.data(), b64_len, data.data(), data.size(), sodium_base64_VARIANT_ORIGINAL);

    // Remove the null terminator added by sodium_bin2base64
    b64_string.pop_back();
    return b64_string;
}

std::vector<uint8_t> crypt::from_base64(const std::string &b64_string) {
    std::vector<uint8_t> data(b64_string.size()); // Base64 decoding will always result in less data than the input size

    size_t data_len;
    if (sodium_base642bin(data.data(), data.size(), b64_string.c_str(), b64_string.size(), nullptr, &data_len, nullptr, sodium_base64_VARIANT_ORIGINAL) != 0) {
        throw std::runtime_error("crypt::from_base64 - Failed to decode base64 string");
    }

    data.resize(data_len); // Resize to actual decoded data length
    return data;
}

keychain crypt::load_keys_from_file() {
    keychain keys{};
    std::ifstream pub_key_file("public.key", std::ios::in | std::ios::binary);
    std::ifstream priv_key_file("private.key", std::ios::in | std::ios::binary);

    if (!pub_key_file.is_open() || !priv_key_file.is_open()) {
        throw std::runtime_error("crypt::load_keys - Failed to open key files");
    }

    // Read the b64 encoded keys from the files
    std::string pub_key_b64((std::istreambuf_iterator<char>(pub_key_file)), std::istreambuf_iterator<char>());
    std::string priv_key_b64((std::istreambuf_iterator<char>(priv_key_file)), std::istreambuf_iterator<char>());

    std::vector<uint8_t> pub_key_vec = from_base64(pub_key_b64);
    std::vector<uint8_t> priv_key_vec = from_base64(priv_key_b64);

    // Ensure the decoded keys are the correct length
    if (pub_key_vec.size() != crypto_sign_PUBLICKEYBYTES || priv_key_vec.size() != crypto_sign_SECRETKEYBYTES) {
        throw std::runtime_error("crypt::load_keys - Decoded keys have invalid length");
    }

    // Copy the decoded keys into the keychain struct
    std::ranges::copy(pub_key_vec, keys.public_key);
    std::ranges::copy(priv_key_vec, keys.private_key);

    return keys;
}

keychain& crypt::get_public_keychain() {
    static keychain pub_keychain{};

    if (pub_keychain.public_key[0] == 0) {
        std::copy(std::begin(public_key), std::end(public_key), pub_keychain.public_key);
    }

    return pub_keychain;
}

keychain crypt::generate_pub_priv_keypair() {
    keychain keys{};

    crypto_sign_keypair(keys.public_key, keys.private_key);

    // Create b64 versions of the keys for easier storage in files
    std::vector<uint8_t> pub_key_vec(keys.public_key, keys.public_key + crypto_sign_PUBLICKEYBYTES);
    std::vector<uint8_t> priv_key_vec(keys.private_key, keys.private_key + crypto_sign_SECRETKEYBYTES);

    std::string pub_key_b64 = to_base64(pub_key_vec);
    std::string priv_key_b64 = to_base64(priv_key_vec);

    std::ofstream pub_key_file("public.key", std::ios::out | std::ios::trunc);
    std::ofstream priv_key_file("private.key", std::ios::out | std::ios::trunc);

    if (!pub_key_file.is_open() || !priv_key_file.is_open()) {
        throw std::runtime_error("crypt::generate_pub_priv_keypair - Failed to open key files for writing");
    }

    // Write them keys
    pub_key_file.write(pub_key_b64.c_str(), pub_key_b64.size());
    priv_key_file.write(priv_key_b64.c_str(), priv_key_b64.size());

    pub_key_file.close();
    priv_key_file.close();

    return keys;
}

std::vector<uint8_t> crypt::create_signature(const std::vector<uint8_t> &data, const keychain &keys) {
    std::vector<uint8_t> signature(crypto_sign_BYTES);

    crypto_sign_detached(signature.data(), nullptr, data.data(), data.size(), keys.private_key);

    return signature;
}

bool crypt::verify_signature(const std::vector<uint8_t> &data, const std::vector<uint8_t> &signature,
    const keychain &keys) {

    return crypto_sign_verify_detached(signature.data(), data.data(), data.size(), keys.public_key) == 0;
}

crypt::crypt() {
    if (sodium_init() < 0) {
        throw std::runtime_error("crypt::crypt - Failed to initialize libsodium");
    }
}
