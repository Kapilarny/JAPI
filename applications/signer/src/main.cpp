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

    try {
        // Load the file to sign
        const std::string file_to_sign = argv[1];
        binary_file file(file_to_sign, false);

        // Load keys
        keychain keys = crypt::get()->load_keys_from_file();

        // Generate signature
        file.generate_signature(keys);
        JINFO("Generated signature for file successfully!");

        // Save the signed file
        const std::string signed_file_path = file_to_sign + ".signed";
        file.save_to_file(signed_file_path, true);
        JINFO("Saved signed file successfully to %s!", signed_file_path.c_str());
    } catch (const std::exception& e) {
        JFATAL("An error occurred: %s", e.what());
        Sleep(3000);
        return 1;
    }

    JINFO("Exiting...");
    Sleep(3000);

    return 0;
}
