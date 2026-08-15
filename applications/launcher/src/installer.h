//
// Created by kapil on 14.08.2026.
//

#ifndef JAPI_INSTALLER_H
#define JAPI_INSTALLER_H
#include <cstdint>
#include <ctime>
#include <vector>

#include "binary_file.h"
#include "downloader.h"
#include "miniz.h"

struct main_manifest {
    std::string japi_version;
    std::time_t timestamp;
};

struct dependencies_manifest {
    std::string japi_version;
    std::vector<std::pair<std::string, std::string>> dependencies; // Pair of dependency name and hash
    std::vector<std::string> load_order; // List of dependency names in the order they should be loaded
};

class installer {
public:
    installer(binary_file& update_package, downloader& dl);

    void install();
private:
    void initialize_archive();
    void shutdown_archive();

    void load_manifests();
    void load_main_manifest();
    void load_dependencies_manifest();

    void check_dependencies();
    void install_files();

    void install_game_binaries();

    std::vector<uint8_t> extract_file_to_vec(mz_uint file_index);
    void extract_file_to_disk(mz_uint file_index, const std::string& output_path);

    static std::string timestamp_to_string(std::time_t timestamp);

    mz_zip_archive _zip{};
    mz_uint _num_files{};
    mz_uint _manifest_file_index{};
    mz_uint _dependencies_file_index{};
    std::vector<mz_uint> _installation_files{};

    binary_file& _update_package;
    downloader& _dl;

    main_manifest _main_manifest{};
    dependencies_manifest _dependencies_manifest{};
};

#endif //JAPI_INSTALLER_H