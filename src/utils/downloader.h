//
// Created by user on 28.12.2024.
//

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "utils/logger.h"

#include <vector>

namespace downloader {
    typedef void (*DownloadCallback)(const std::vector<uint8_t>& buffer, void* user_data);

    void download_file_async(const std::string& url, DownloadCallback callback, void* user_data = nullptr);

    bool is_404(std::vector<uint8_t>& buffer);
    std::vector<uint8_t> download_file(std::string url);
}
#endif //DOWNLOADER_H
