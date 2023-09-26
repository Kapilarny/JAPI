#include "parser.h"

namespace Parser {
    IParserType::IParserType(std::vector<uint8_t> origBytes) {
        parseXfbin(origBytes);
    }

    void IParserType::parseXfbin(std::vector<uint8_t> bytes) {
        // Do nothing
    }
} // namespace Parser