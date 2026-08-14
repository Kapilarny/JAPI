//
// Created by kapil on 13.08.2026.
//

#include "version.h"

#include <stdexcept>

version::version(const std::string &str) {
    // Parse the version string in the format "major.minor.patch"
    size_t first_dot = str.find('.');
    size_t second_dot = str.find('.', first_dot + 1);

    if (first_dot == std::string::npos || second_dot == std::string::npos) {
        throw std::runtime_error("version::version: Invalid version string format");
    }

    major = static_cast<uint8_t>(std::stoi(str.substr(0, first_dot)));
    minor = static_cast<uint8_t>(std::stoi(str.substr(first_dot + 1, second_dot - first_dot - 1)));
    patch = static_cast<uint8_t>(std::stoi(str.substr(second_dot + 1)));
}

version::version(const uint8_t major, const uint8_t minor, const uint8_t patch) {
    this->major = major;
    this->minor = minor;
    this->patch = patch;
}

std::string version::to_string() const {
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

bool version::operator==(const version & b) const {
    return major == b.major && minor == b.minor && patch == b.patch;
}
