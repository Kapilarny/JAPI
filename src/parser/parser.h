#include <iostream>
#include <fstream>
#include <cstdint>
#include <bit>
#include <unordered_map>

#include <toml.hpp>

#define IS_BIG_ENDIAN (std::endian::native == std::endian::big)

namespace Parser {
    class IParserType {
    public:
        IParserType(std::vector<uint8_t> origBytes);

        virtual void parse(std::vector<uint8_t> bytes);
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
    T toBigEndian(T value) {
        if (!IS_BIG_ENDIAN) return _byteswap_ulong(value); else return value;
    }
    template <typename T>
    T toLittleEndian(T value) {
        if (IS_BIG_ENDIAN) return _byteswap_ulong(value); else return value;
    }

    void LittleEndian();
    void BigEndian();

    template <typename T>
    T parse(std::istream& file, T param) {
        T buffer;
        file.read((char*)&buffer, sizeof(buffer));
        if (typeid(buffer).name() == typeid(int).name()) {
            if (endianType == Endian::BIG) {
                buffer = toBigEndian(buffer);
            } else {
                buffer = toLittleEndian(buffer);
            }
        }
        return buffer;
    }

    std::string parseStr(std::istream& file);

    std::string swapBytes(std::string input);
} // namespace Parser