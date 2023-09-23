#pragma once

#include <string>
#include <unordered_map>

#include "parser.h"
#include "json.hpp"

struct MessageInfoJSONEntry {
    int priority;
    std::string message;
    uint16_t is_ref;
    std::string ref_crc32;
    uint16_t char_id;
    uint16_t cue_id;
};

class MessageInfo : public Parser::IParserType {
public:
    MessageInfo(std::vector<uint8_t> origBytes) : IParserType(origBytes) {}

    void parseXfbin(std::vector<uint8_t> bytes) override;
    void merge(json data) override;
    std::vector<uint8_t> serialize() override;
private:
    std::unordered_map<std::string, MessageInfoJSONEntry> entries;

    uint32_t version;
    uint32_t count;
    uint32_t size;

    std::vector<uint8_t> header;
    std::vector<uint8_t> footer;
};