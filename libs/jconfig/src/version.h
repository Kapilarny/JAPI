//
// Created by kapil on 13.08.2026.
//

#ifndef JAPI_VERSION_H
#define JAPI_VERSION_H
#include <cstdint>
#include <string>

struct version {
    explicit version(const std::string& str);
    version(uint8_t major, uint8_t minor, uint8_t patch);

    [[nodiscard]] std::string to_string() const;

    uint8_t major{}, minor{}, patch{};

    std::strong_ordering operator<=>(const version & version) const;
    bool operator==(const version &) const;
};

#endif //JAPI_VERSION_H