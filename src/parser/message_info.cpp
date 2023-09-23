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
    header = std::vector<uint8_t>(bytes.begin(), bytes.begin() + 284);
    footer = std::vector<uint8_t>(bytes.end() - 20, bytes.end());
    size_t offset = 284; // Skipping the header fluff

    LittleEndian();    
    offset = parseBytes(bytes, &size, offset);

    size = toLittleEndian(size); 
    printf("Size: %d\n", size);

    uintptr_t firstPtr;
    offset = parseBytes(bytes, &version, offset);
    offset = parseBytes(bytes, &count, offset);
    offset = parseBytes(bytes, &firstPtr, offset);

    size_t messageSize = 0;
    for(int i = 0; i < count; i++) {
        MessageInfoEntry entry;
        uint32_t crc32;
        offset = parseBytes(bytes, &crc32, offset);
        entry.crc32_id = std::format("{:08X}", crc32);

        offset = parseBytes(bytes, &entry.unk1, offset);
        offset = parseBytes(bytes, &entry.unk2, offset);
        offset = parseBytes(bytes, &entry.unk3, offset);

        uint64_t ptr;
        parseBytes(bytes, &ptr, offset);
        parseBytes(bytes, &entry.message, offset + ptr);

        messageSize += entry.message.size() + 1;

        offset += 8; // Skip "ptr"

        crc32 = 0;
        offset = parseBytes(bytes, &crc32, offset);
        entry.ref_crc32 = std::format("{:08X}", crc32);

        offset = parseBytes(bytes, &entry.is_ref, offset);
        offset = parseBytes(bytes, &entry.char_id, offset);
        offset = parseBytes(bytes, &entry.cue_id, offset);

        offset = parseBytes(bytes, &entry.unk6, offset);
        offset = parseBytes(bytes, &entry.unk7, offset);

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
    size = 20 + messageSize + 40 * count;
}

std::vector<uint8_t> MessageInfo::serialize() {
    uint64_t pointer = 8;

    // Calculate the size
    int saveSize = size; // to be used later
    while (size % 4 != 0) {
        size++;
    }

    // Reserve the size
    std::vector<uint8_t> result = std::vector<uint8_t>(size + header.size() + footer.size() + 12, 0);

    // Copy the header
    result.insert(result.end(), header.begin(), header.end());

    uint16_t b16; // buffer16
    uint32_t b32; // buffer32
    uint64_t b64; // buffer64
    uint64_t bp; // buffer pointer

    size = toBigEndian(size - 4);

    size_t offset = header.size();
    uint32_t totalSize = toBigEndian(toBigEndian(size) + 4);
    
    // Some XFBIN magic values
    writeData(result, totalSize, &offset);

    b32 = toBigEndian(1);
    writeData(result, b32, &offset);

    b32 = toBigEndian(7956736); // magic number haha
    writeData(result, b32, &offset);

    // XFBIN header data
    writeData(result, size, &offset);
    writeData(result, version, &offset);
    writeData(result, count, &offset);
    writeData(result, pointer, &offset);

    // Loop through the table
    bp = 40 * count - 16;
    for(auto [key, value] : entries) {
        // Get the crc32
        b32 = std::stoul(key.c_str(), nullptr, 16);
        writeData(result, b32, &offset);

        // Write the unknowns
        b32 = 0;
        writeData(result, b32, &offset);

        b64 = 0;
        writeData(result, b64, &offset);

        // Write the message pointer
        writeData(result, bp, &offset);

        // Write the string
        std::copy(value.message.begin(), value.message.end(), result.begin() + header.size() + bp);
        result[header.size() + bp + value.message.size()] = 0;

        bp += value.message.size() + 1 - 40;

        // ref_crc32
        b32 = std::stoul(value.ref_crc32.c_str(), nullptr, 16);
        b32 = toBigEndian(b32);
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

    while(saveSize % 4 != 0) {
        result[saveSize] = 0;
        saveSize++;
    }

    // Copy the footer
    result.insert(result.end() - footer.size(), footer.begin(), footer.end());

    return result;
}

void MessageInfo::merge(json data) {
    MessageInfoJSONEntry entry;
    entry.priority = data["priority"];

    if(entries.contains(data["crc32_id"])) {
        if(entries[data["crc32_id"]].priority < entry.priority) {
            return;
        }
    }

    entry.message = data["message"];
    entry.is_ref = data["is_ref"];
    entry.ref_crc32 = data["ref_crc32"];
    entry.char_id = data["char_id"];
    entry.cue_id = data["cue_id"];

    entries[data["crc32_id"]] = entry;
}