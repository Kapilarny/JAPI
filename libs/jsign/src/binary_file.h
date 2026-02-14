//
// Created by kapil on 13.02.2026.
//

#ifndef JAPI_PRELOAD_BINARY_FILE_H
#define JAPI_PRELOAD_BINARY_FILE_H
#include <cstdint>
#include <string>
#include <vector>

#include "crypt.h"

class binary_file {
public:
    binary_file(const std::string& path, bool is_signed = false);
    binary_file(std::vector<uint8_t>& data, bool is_signed = false);

    void assign_signature(const std::vector<uint8_t>& signature);
    void generate_signature(const keychain& keys);

    bool verify_signature() const;

    void save_to_file(const std::string& path, bool with_signature);

    [[nodiscard]] const std::vector<uint8_t>& get_data() const { return _data; }
    [[nodiscard]] const std::vector<uint8_t>& get_signature() const { return _signature; }

    [[nodiscard]] std::vector<uint8_t> generate_signed_file();
private:
    void load(std::vector<uint8_t>& data, bool is_signed);

    std::vector<uint8_t> _data{};
    std::vector<uint8_t> _signature{};
    bool _is_signed = false;
};

#endif //JAPI_PRELOAD_BINARY_FILE_H