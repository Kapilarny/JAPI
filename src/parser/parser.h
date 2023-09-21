#include <iostream>
#include <fstream>
#include <cstdint>
#include <bit>
#include <unordered_map>
#include <algorithm>

#include <toml.hpp>

#define IS_BIG_ENDIAN (std::endian::native == std::endian::big)

namespace Parser {
    class IParserType {
    public:
        IParserType(std::vector<uint8_t> origBytes);

        virtual void parseXfbin(std::vector<uint8_t> bytes);
        virtual void merge(toml::table data, int priority) = 0;
        virtual std::vector<uint8_t> serialize() = 0;
    protected:
        toml::table table;
    };

    enum class Endian {
        LITTLE,
        BIG
    };

    inline Endian endianType = Endian::LITTLE;

    template <typename T>
    T SwapEndianess(T value) {
        std::reverse(&value, &value + sizeof(value));

        return value;
    }

    template <typename T>
    T toBigEndian(T value) {
        if (!IS_BIG_ENDIAN) return SwapEndianess(value); else return value;
    }
    
    template <typename T>
    T toLittleEndian(T value) {
        if (IS_BIG_ENDIAN) return SwapEndianess(value); else return value;
    }

    void LittleEndian();
    void BigEndian();

    template <typename T>
    T parse(std::istream& file, size_t offset = 0) {
        T buffer;
        file.read((char*)&buffer, sizeof(buffer));
        
        if (endianType == Endian::BIG) {
            buffer = toBigEndian(buffer);
        } else {
            buffer = toLittleEndian(buffer);
        }

        return buffer;
    }

    template <typename T>
    size_t parseBytes(std::vector<uint8_t> data, T* returnData, size_t offset) {
        std::copy(data.begin() + offset, data.begin() + offset + sizeof(T), (uint8_t*)returnData);

        if (endianType == Endian::BIG) {
            *returnData = toBigEndian(*returnData);
        } else {
            *returnData = toLittleEndian(*returnData);
        }

        return offset + sizeof(T);
    }

    std::string parseStr(std::istream& file);

    std::string swapBytes(std::string input);
} // namespace Parser