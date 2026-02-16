//
// Created by kapil on 13.02.2026.
//

#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "crypt.h"
#include "logger.h"
#include "binary_file.h"

// atexit for whatever reason won't work
// idfk whats going on

int main(int argc, char* argv[]) {
    logger::init("JAPISigner", "japi/logs");

    if (argc < 2) {
        JWARN("No file specified to sign, do you want to generate a keypair? (y/n)");

        try {
            char response;
            std::cin >> response;
            if (response == 'y' || response == 'Y') {
                crypt::get()->generate_pub_priv_keypair();
                JINFO("Generated keypair successfully! Public key saved to public.key, private key saved to private.key");
            }

        } catch (const std::exception& e) {
            JFATAL("An error occurred: %s", e.what());
            Sleep(3000);
            return 1;
        }

        JINFO("Exiting...");
        Sleep(3000);
        return 0;
    }

    std::string key_directory;
    if (argc >= 3) {
        key_directory = argv[2];
        if (key_directory.back() != '\\' && key_directory.back() != '/') {
            key_directory += '\\';
        }
    }

    JINFO ("Loading keys from directory: %s", key_directory.c_str());

    try {
        // Load keys
        keychain keys = crypt::get()->load_keys_from_file(key_directory);

        // Load the file to sign
        const std::string file_to_sign = argv[1];

        // If directory is provided, sign all files in the directory
        if (std::filesystem::is_directory(file_to_sign)) {
            JINFO("Signing all files in directory: %s", file_to_sign.c_str());

            // Iterate through all files in the directory and sign them
            for (const auto& entry : std::filesystem::recursive_directory_iterator(file_to_sign)) {
                if (entry.is_regular_file()) {
                    JINFO("Signing file: %s", entry.path().string().c_str());
                    binary_file file(entry.path().string(), false);
                    file.generate_signature(keys);

                    // The path is: file_to_sign + "/signed/" + relative_path + original_file_name + ".signed"
                    const std::string relative_path = std::filesystem::relative(entry.path(), file_to_sign).parent_path().string();
                    const std::string signed_file_path = file_to_sign + "signed\\" + relative_path + "\\" + entry.path().filename().string() + ".signed";

                    // Create the signed directory if it doesn't exist
                    std::filesystem::create_directories(std::filesystem::path(signed_file_path).parent_path());

                    file.save_to_file(signed_file_path, true);
                    JINFO("Saved signed file successfully to %s!", signed_file_path.c_str());
                }
            }
        } else {
            binary_file file(file_to_sign, false);

            file.generate_signature(keys);
            JINFO("Generated signature for file successfully!");

            const std::string signed_file_path = file_to_sign + ".signed";
            file.save_to_file(signed_file_path, true);
            JINFO("Saved signed file successfully to %s!", signed_file_path.c_str());
        }
    } catch (const std::exception& e) {
        JFATAL("An error occurred: %s", e.what());
        Sleep(3000);
        return 1;
    }

    JINFO("Exiting...");
    Sleep(3000);

    return 0;
}
