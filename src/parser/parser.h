#include <iostream>
#include <fstream>
#include <cstdint>
#include <bit>
#include <unordered_map>
#include <algorithm>

#include <toml.hpp>
#include "json.hpp"

using json = nlohmann::json;

namespace Parser {
    class IParserType {
    public:
        IParserType(std::vector<uint8_t> origBytes);

        virtual void parseXfbin(std::vector<uint8_t> bytes);
        virtual void merge(json data) = 0;
        virtual std::vector<uint8_t> serialize() = 0;
    };

    template <typename T>
    inline T SwapEndianess(T value) {
        uint8_t* bytes = (uint8_t*)&value;
        std::reverse(bytes, bytes + sizeof(T));

        return value;
    }

    template <>
    inline std::string SwapEndianess(std::string value) {
        std::reverse(value.begin(), value.end());

        return value;
    }

    template <typename T>
    inline void writeData(const std::vector<uint8_t>& data, T value, size_t* offset) {
        memcpy((uint8_t*)data.data() + *offset, &value, sizeof(value));
        *offset += sizeof(value);
    }

    template <typename T>
    inline void parseBytes(const std::vector<uint8_t>& data, T* returnData, size_t* offset) {
        memcpy(returnData, (uint8_t*)data.data() + *offset, sizeof(T));
        *offset += sizeof(T);
    }

    template <>
    inline void parseBytes(const std::vector<uint8_t>& data, std::string* returnData, size_t* offset) {
        // Create a buffer
        std::vector<uint8_t> buffer;

        // Copy the bytes until we hit a null terminator
        while (data[*offset] != 0) {
            buffer.push_back(data[*offset]);
            *offset += 1;
        }

        // Copy the buffer to the string
        returnData->resize(buffer.size());

        memcpy((uint8_t*)returnData->data(), buffer.data(), buffer.size());
        *offset += buffer.size() + 1;
    }
} // namespace Parser