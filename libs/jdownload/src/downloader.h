//
// Created by kapil on 13.02.2026.
//

#ifndef JAPI_PRELOAD_DOWNLOADER_H
#define JAPI_PRELOAD_DOWNLOADER_H
#include <string>
#include <vector>
#include <windows.h>
#include <winhttp.h>

class downloader {
public:
    downloader();

    std::vector<char> download_file(const std::string& url);
    void download_to_disk(const std::string& url, const std::string& output_path);
private:
    HINTERNET _internet;
};

#endif //JAPI_PRELOAD_DOWNLOADER_H