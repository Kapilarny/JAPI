#include "message_info.h"

#include <format>

using namespace Parser;

void MessageInfo::parse(std::vector<uint8_t> bytes) {
    // Parse the original file here
}

std::vector<uint8_t> MessageInfo::serialize() {
    // Serialize the table here

    return std::vector<uint8_t>();
}

void MessageInfo::merge(toml::table data, int priority) {
    // Get the crc32_id from the table
    std::string crc32_id = data["crc32_id"].value_or("null");

    // Check if the crc32_id is null
    if (crc32_id == "null") return;

    // Check if the crc32_id is already in the table
    if (table.contains(crc32_id)) {
        int existingPriority = table[crc32_id]["priority"].value_or(0);
        if(existingPriority > priority) return;
    }
    
    table.insert_or_assign(crc32_id, data);
}