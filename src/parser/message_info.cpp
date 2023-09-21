#include "message_info.h"

#include <format>

using namespace Parser;
using namespace nlohmann;

struct MessageInfoEntry {
    std::string crc32_id;

    uint32_t unk1; // Hidden: unimportant
    uint32_t unk2; // Hidden: unimportant
    uint32_t unk3; // Hidden: unimportant

    std::string message;

    std::string ref_crc32;
    int16_t is_ref;

    int16_t char_id;
    int16_t cue_id;

    int16_t unk6; // Hidden: unimportant
    uint32_t unk7; // Hidden: unimportant
};

void MessageInfo::parseXfbin(std::vector<uint8_t> bytes) {
    size_t offset = 284; // Skipping the header fluff

    LittleEndian();
    uint32_t size;
    offset = parseBytes(bytes, &size, offset);
    
    uint32_t version, count, firstPtr;
    offset = parseBytes(bytes, &version, offset);
    offset = parseBytes(bytes, &count, offset);
    offset = parseBytes(bytes, &firstPtr, offset);

    table.insert_or_assign("Version", version);

    for(int i = 0; i < count; i++) {
        MessageInfoEntry entry;

        offset = parseBytes(bytes, &entry.crc32_id, offset);
        std::string hex_output = std::format("{:08X}", entry.crc32_id);
        while(hex_output.size() < 8) {
            hex_output = "0" + hex_output;
        }
        entry.crc32_id = hex_output;

        offset = parseBytes(bytes, &entry.unk1, offset);
        offset = parseBytes(bytes, &entry.unk2, offset);
        offset = parseBytes(bytes, &entry.unk3, offset);

        uint64_t ptr;
        parseBytes(bytes, &ptr, offset);
        parseBytes(bytes, &entry.message, offset + ptr);

        offset += 8; // Skip "ptr"

        hex_output = std::format("{:08X}", entry.ref_crc32);
        while(hex_output.size() < 8) {
            hex_output = "0" + hex_output;
        }
        entry.ref_crc32 = hex_output;

        offset = parseBytes(bytes, &entry.is_ref, offset);
        offset = parseBytes(bytes, &entry.char_id, offset);
        offset = parseBytes(bytes, &entry.cue_id, offset);

        offset = parseBytes(bytes, &entry.unk6, offset);
        offset = parseBytes(bytes, &entry.unk7, offset);

        toml::table entryTable;
        entryTable.insert("message", entry.message);
        entryTable.insert("is_ref", entry.is_ref);
        entryTable.insert("ref_crc32", entry.ref_crc32);
        entryTable.insert("char_id", entry.char_id);
        entryTable.insert("cue_id", entry.cue_id);

        table.insert_or_assign(entry.crc32_id, entryTable);
    }
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