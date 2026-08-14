//
// Created by kapil on 14.08.2026.
//

#include "installer.h"

#include <algorithm>
#include <fstream>
#include <logger.h>
#include <miniz.h>

#include "json.hpp"

using json = nlohmann::json;

installer::installer(binary_file &update_package) : _update_package(update_package) {}

void installer::install() {
    JINFO("Trying to install update package...");

    // Verify the update package signature
    if (_update_package.verify_signature()) {
        JINFO("Update package signature verified successfully.");
    } else {
        throw std::runtime_error("installer::install - Tampered/corrupted update package detected.");
    }

    initialize_archive();

    load_manifests();
    check_dependencies();
    install_files();

    shutdown_archive();
}

void installer::load_manifests() {
    load_main_manifest();
    load_dependencies_manifest();

    // Check mismatch
    if (_main_manifest.japi_version != _dependencies_manifest.japi_version) {
        throw std::runtime_error("installer::load_manifests - Version mismatch between main manifest and dependencies manifest.");
    }
}


void installer::initialize_archive() {
    const auto& data = _update_package.get_data();

    if (!mz_zip_reader_init_mem(&_zip, data.data(), data.size(), 0)) {
        throw std::runtime_error("installer::initialize_archive - Failed to initialize zip reader.");
    }

    _num_files = mz_zip_reader_get_num_files(&_zip);
    JINFO("%u files found in the update package.", _num_files);

    for (mz_uint i = 0; i < _num_files; i++) {
        mz_zip_archive_file_stat stat{};

        if (!mz_zip_reader_file_stat(&_zip, i, &stat))
            continue;

        const auto filename = std::string(stat.m_filename);

        if (filename == "manifest.json") {
            _manifest_file_index = i;
        } else if (filename == "dependencies.json") {
            _dependencies_file_index = i;
        } else {
            _installation_files.push_back(i);
        }
    }

    if (!_manifest_file_index || !_dependencies_file_index) {
        throw std::runtime_error("installer::initialize_archive - manifest.json or dependencies.json file not found in the update package.");
    }
}

void installer::shutdown_archive() {
    mz_zip_reader_end(&_zip);
}

std::vector<uint8_t> installer::extract_file_to_vec(const mz_uint file_index) {
    mz_zip_archive_file_stat stat{};
    if (!mz_zip_reader_file_stat(&_zip, file_index, &stat)) {
        throw std::runtime_error("installer::extract_file - Failed to get file stat for index " + std::to_string(file_index));
    }

    std::vector<uint8_t> buffer(stat.m_uncomp_size);
    if (!mz_zip_reader_extract_to_mem(&_zip, file_index, buffer.data(), buffer.size(), 0)) {
        throw std::runtime_error("installer::extract_file - Failed to extract file at index " + std::to_string(file_index));
    }

    return buffer;
}

void installer::extract_file_to_disk(const mz_uint file_index, const std::string& output_path) {
    mz_zip_archive_file_stat stat{};
    if (!mz_zip_reader_file_stat(&_zip, file_index, &stat)) {
        throw std::runtime_error("installer::extract_file_to_disk - Failed to get file stat for index " + std::to_string(file_index));
    }

    // Ensure the output directory exists
    std::filesystem::path output_dir = std::filesystem::path(output_path).parent_path();
    if (!std::filesystem::exists(output_dir)) {
        std::filesystem::create_directories(output_dir);
    }

    if (!mz_zip_reader_extract_to_file(&_zip, file_index, output_path.c_str(), 0)) {
        throw std::runtime_error("installer::extract_file_to_disk - Failed to extract file at index " + std::to_string(file_index) + " to disk at path: " + output_path);
    }
}


void installer::load_main_manifest() {
    const auto manifest_data = extract_file_to_vec(_manifest_file_index);

    json manifest_json = json::parse(manifest_data);
    if (!manifest_json.contains("manifest_version") || !manifest_json.contains("content")) {
        throw std::runtime_error("installer::load_manifest - Invalid manifest.json structure.");
    }

    const uint32_t manifest_version = manifest_json["manifest_version"];
    if (manifest_version != 1) {
        throw std::runtime_error("installer::load_manifest - Unsupported manifest version: " + std::to_string(manifest_version));
    }

    const auto& content = manifest_json["content"];
    const std::string japi_version = content["version"];
    const time_t timestamp = content["timestamp"];

    _main_manifest = { japi_version, timestamp };

    const std::string timestamp_str = timestamp_to_string(timestamp);
    JINFO("Manifest loaded successfully. Version: %s, Date: %s", japi_version.c_str(), timestamp_str.c_str());
}

void installer::load_dependencies_manifest() {
    const auto dependencies_data = extract_file_to_vec(_dependencies_file_index);

    json dependencies_json = json::parse(dependencies_data);
    if (!dependencies_json.contains("manifest_version") || !dependencies_json.contains("content")) {
        throw std::runtime_error("installer::load_dependencies_manifest - Invalid dependencies.json structure.");
    }

    const uint32_t manifest_version = dependencies_json["manifest_version"];
    if (manifest_version != 1) {
        throw std::runtime_error("installer::load_dependencies_manifest - Unsupported manifest version: " +std::to_string(manifest_version));
    }

    const auto& content = dependencies_json["content"];

    _dependencies_manifest = { content["version"], {}, {} };

    // Load dependencies
    for (const auto& dep : content["dlls"]) {
        const std::string dep_name = dep["name"];
        const std::string dep_hash = dep["hash"];
        _dependencies_manifest.dependencies.emplace_back(dep_name, dep_hash);
    }

    // Load load order
    for (const auto& order : content["load_order"]) {
        _dependencies_manifest.load_order.push_back(order);
    }
}

void installer::check_dependencies() {
    std::vector<std::string> missing_dependencies;

    // Check the japi/dlls/libs for missing/mismatched dependencies based on the dependencies manifest
    for (const auto& [dep_name, dep_hash_str] : _dependencies_manifest.dependencies) {
        const std::filesystem::path dep_path = std::filesystem::path("japi/dlls/libs") / dep_name;

        if (!std::filesystem::exists(dep_path)) {
            JWARN("Missing dependency: %s", dep_name.c_str());
            missing_dependencies.push_back(dep_name);
            continue;
        }

        // Verify the hash of the existing dependency
        binary_file dep_file(dep_path.string(), false);
        auto f_hash = dep_file.generate_hash();
        auto dep_hash = sha256hash(dep_hash_str);

        if (f_hash != dep_hash) {
            JWARN("Mismatched hash for dependency: %s", dep_name.c_str());
            missing_dependencies.push_back(dep_name);
        }
    }

    for (const auto& dep_name : missing_dependencies) {
        JINFO("Acquiring missing dependency: %s", dep_name.c_str());

        // TODO: Download the missing dependency

    }

    // Generate lib_load_order.txt
    std::string content = "# This file is auto-generated by the JAPI Launcher.\n# DO NOT MODIFY UNLESS YOU KNOW WHAT YOU ARE DOING.\n";
    for (const auto& dep_name : _dependencies_manifest.load_order) {
        content += dep_name + "\n";
    }

    std::filesystem::path load_order_path = std::filesystem::path("japi/dlls") / "lib_load_order.txt";
    std::ofstream load_order_file(load_order_path);
    if (!load_order_file.is_open()) {
        throw std::runtime_error("installer::check_dependencies - Failed to open lib_load_order.txt for writing.");
    }
    load_order_file << content;
    load_order_file.close();
}

void installer::install_files() {
    for (const auto& file_index : _installation_files) {
        mz_zip_archive_file_stat stat{};
        if (!mz_zip_reader_file_stat(&_zip, file_index, &stat)) {
            JWARN("Failed to get file stat for index %u. Skipping.", file_index);
            continue;
        }

        const std::string output_path = stat.m_filename;
        JINFO("Installing file: %s", output_path.c_str());

        // Make directories
        std::filesystem::path output_dir = std::filesystem::path(output_path).parent_path();
        if (!std::filesystem::exists(output_dir)) {
            std::filesystem::create_directories(output_dir);
        }

        try {
            extract_file_to_disk(file_index, output_path);
        } catch (const std::exception& e) {
            JWARN("Failed to install file %s: %s", output_path.c_str(), e.what());
        }

        JINFO("Successfully installed file: %s", output_path.c_str());
    }
}

std::string installer::timestamp_to_string(const std::time_t timestamp) {
    const std::tm tm = *std::gmtime(&timestamp);

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    return ss.str();
}
