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
    // Copy the header and footer
    header.reserve(284);
    footer.reserve(20);

    // Copy the first 284 bytes to the header
    std::copy(bytes.begin(), bytes.begin() + 284, std::back_inserter(header));

    // Copy the last 20 bytes to the footer
    std::copy(bytes.end() - 20, bytes.end(), std::back_inserter(footer)); 

    size_t offset = 288; // Skipping the header fluff

    uintptr_t firstPtr;
    parseBytes(bytes, &version, &offset);
    parseBytes(bytes, &count, &offset);
    parseBytes(bytes, &firstPtr, &offset);

    size_t messageSize = 0;
    for(int i = 0; i < count; i++) {
        MessageInfoEntry entry;
        uint32_t crc32;
        parseBytes(bytes, &crc32, &offset);
        entry.crc32_id = std::format("{:08X}", crc32);

        parseBytes(bytes, &entry.unk1, &offset);
        parseBytes(bytes, &entry.unk2, &offset);
        parseBytes(bytes, &entry.unk3, &offset);

        uint64_t ptr;
        parseBytes(bytes, &ptr, &offset);

        size_t ptrOffset = offset + ptr - 8;
        parseBytes(bytes, &entry.message, &ptrOffset);

        messageSize += entry.message.size() + 1;

        crc32 = 0;
        parseBytes(bytes, &crc32, &offset);
        entry.ref_crc32 = std::format("{:08X}", crc32);

        parseBytes(bytes, &entry.is_ref, &offset);
        parseBytes(bytes, &entry.char_id, &offset);
        parseBytes(bytes, &entry.cue_id, &offset);

        parseBytes(bytes, &entry.unk6, &offset);
        parseBytes(bytes, &entry.unk7, &offset);

        MessageInfoJSONEntry jsonEntry;
        jsonEntry.priority = 0;
        jsonEntry.message = entry.message;
        jsonEntry.is_ref = entry.is_ref;
        jsonEntry.ref_crc32 = entry.ref_crc32;
        jsonEntry.char_id = entry.char_id;
        jsonEntry.cue_id = entry.cue_id;

        entries[entry.crc32_id] = jsonEntry;
    }

    // Calculate size
    size = 20 + messageSize + 40 * count - 3;
}

std::vector<uint8_t> MessageInfo::serialize() {
    uint64_t pointer = 8;

    // Reserve the size
    std::vector<uint8_t> result = std::vector<uint8_t>(size + header.size() + footer.size() + 4, 0);

    // Write the header
    std::copy(header.begin(), header.end(), result.begin());

    uint16_t b16; // buffer16
    uint32_t b32; // buffer32
    uint64_t b64; // buffer64
    uint64_t bp; // buffer pointer

    size_t offset = 284;

    size = SwapEndianess(size); // Swap to big endian

    // XFBIN header data
    writeData(result, size, &offset);
    writeData(result, version, &offset);
    writeData(result, count, &offset);
    writeData(result, pointer, &offset);

    uint32_t totalMessageSize = 0;
    size_t startEntryOffset = offset;
    size_t entriesSize = 40 * count;
    for(auto [key, value] : entries) {
        // Get the crc32
        b32 = std::stoul(key.c_str(), nullptr, 16);
        writeData(result, b32, &offset);

        // Write the unknowns
        b32 = 0;
        writeData(result, b32, &offset);

        b64 = 0;
        writeData(result, b64, &offset);

        // Calculate the bp
        bp = startEntryOffset + entriesSize + totalMessageSize;

        // Write the message
        memcpy(result.data() + bp, value.message.c_str(), value.message.size() + 1);
        totalMessageSize += value.message.size() + 1;

        // Reajust the bp
        bp -= offset;

        // Write the message pointer
        writeData(result, bp, &offset);

        // ref_crc32
        b32 = std::stoul(value.ref_crc32.c_str(), nullptr, 16);
        // b32 = toBigEndian(b32);
        writeData(result, b32, &offset);
    
        // is_ref
        writeData(result, value.is_ref, &offset);
        
        // char_id
        writeData(result, value.char_id, &offset);

        // cue_id
        writeData(result, value.cue_id, &offset);

        // unknowns
        b16 = 0;
        writeData(result, b16, &offset);

        b32 = 0;
        writeData(result, b32, &offset);
    }

    // Copy the footer
    memcpy(result.data() + result.size() - footer.size(), footer.data(), footer.size());

    return result;
}

void MessageInfo::merge(json data) {
    MessageInfoJSONEntry entry;
    entry.priority = data["priority"];

    auto crc = data["crc32_id"];
    if(entries.contains(crc)) {
        if(entries[crc].priority < entry.priority) return;

        size -= entries[crc].message.size() + 1;
    } else {
        count++;
    }

    entry.message = data["message"];
    entry.is_ref = data["is_ref"];
    entry.ref_crc32 = data["ref_crc32"];
    entry.char_id = data["char_id"];
    entry.cue_id = data["cue_id"];

    // update size
    size += entry.message.size() + 1;

    entries[crc] = entry;
}