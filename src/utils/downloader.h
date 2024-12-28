//
// Created by user on 28.12.2024.
//

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "utils/logger.h"

#include <vector>

namespace downloader {
    bool is_404(std::vector<uint8_t>& buffer);
    std::vector<uint8_t> download_file(std::string url);
}
#endif //DOWNLOADER_H
