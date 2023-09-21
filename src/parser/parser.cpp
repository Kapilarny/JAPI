#include "parser.h"

namespace Parser {
    IParserType::IParserType(std::vector<uint8_t> origBytes) {
        parseXfbin(origBytes);
    }

    void IParserType::parseXfbin(std::vector<uint8_t> bytes) {
        // Do nothing
    }

    void LittleEndian() {
        endianType = Endian::LITTLE;
    }

    void BigEndian() {
        endianType = Endian::BIG;
    }

    std::string swapBytes(std::string input) {
        int size = input.size() / 2;
        std::string output;
        for (int i = size; i > 0; i--) {
            output += input.at(i * 2 - 2);
            output += input.at(i * 2 - 1);
        }
        return output;
    }

    std::string parseStr(std::istream& file) {
        char buffer;
        std::string output;
        while (file.peek() != 0) {
            file.read((char*)&buffer, 1);
            output += buffer;
        }
        return output;
    }

    void XFBIN(std::istream& input, std::ostream& output) {
        char xfbinByte;
        input.seekg(0, std::ios::end);
        int size = input.tellg();
        input.seekg(0, std::ios::beg);
        for (int i = 0; i < size; i++) {
            input.read((&xfbinByte), 1);
            output.put(xfbinByte);
        }
    }
} // namespace Parser