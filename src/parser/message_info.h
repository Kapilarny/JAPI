#pragma once

#include "parser.h"
#include "json.hpp"

class MessageInfo : public Parser::IParserType {
public:
    MessageInfo(std::vector<uint8_t> origBytes) : IParserType(origBytes) {}

    void parseXfbin(std::vector<uint8_t> bytes) override;
    void merge(toml::table data, int priority) override;
    std::vector<uint8_t> serialize() override;
};