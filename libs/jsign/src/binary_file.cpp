//
// Created by kapil on 13.02.2026.
//

#include "binary_file.h"

#include <fstream>

#include "crypt.h"

binary_file::binary_file(const std::string &path, bool is_signed) {
    // Open the file in binary mode and read it into a char vector
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("binary_file::binary_file - Failed to open file: " + path);
    }

    // Read the file into a vector
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    load(data, is_signed);
}

binary_file::binary_file(std::vector<uint8_t> &data, bool is_signed) {
    load(data, is_signed);
}

void binary_file::assign_signature(const std::vector<uint8_t> &signature) {
    _signature = signature;
    _is_signed = true;
}

void binary_file::generate_signature(const keychain &keys) {
    if (_is_signed) {
        throw std::runtime_error("binary_file::generate_signature - Trying to generate signature for a file that is already signed!");
    }

    _signature = crypt::get()->create_signature(_data, keys);
    _is_signed = true;
}

bool binary_file::verify_signature() const {
    if (!_is_signed) {
        throw std::runtime_error("binary_file::verify_signature - Trying to verify signature of a file that is not signed!");
    }

    return crypt::get()->verify_signature(_data, _signature, crypt::get()->get_public_keychain());
}

void binary_file::save_to_file(const std::string &path, bool with_signature) {
    std::vector<uint8_t> file_data;
    if (with_signature && _is_signed) {
        file_data = generate_signed_file();
    } else {
        file_data = _data;
    }

    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("binary_file::save_to_file - Failed to open file for writing: " + path);
    }

    file.write(reinterpret_cast<const char *>(file_data.data()), file_data.size());
    file.close();
}

std::vector<uint8_t> binary_file::generate_signed_file() {
    if (!_is_signed) {
        throw std::runtime_error("binary_file::generate_signed_file - Trying to generate signed file from a file that is not signed!");
    }

    std::vector<uint8_t> signed_file;
    signed_file.reserve(_data.size() + _signature.size());
    signed_file.insert(signed_file.end(), _data.begin(), _data.end());
    signed_file.insert(signed_file.end(), _signature.begin(), _signature.end());

    return signed_file;
}

void binary_file::load(std::vector<uint8_t> &data, bool is_signed) {
    if (is_signed) {
        _signature = std::vector<uint8_t>(data.end() - crypto_sign_BYTES, data.end());
        _data = std::vector<uint8_t>(data.begin(), data.end() - crypto_sign_BYTES);
        _is_signed = true;
    } else {
        _is_signed = false;
        _data = std::move(data);
    }
}
