//
// Created by kapil on 13.02.2026.
//

#include "downloader.h"

#include <iostream>
#include <stdexcept>

downloader::downloader() {
    _internet = WinHttpOpen(
        L"JAPIDownloader/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (!_internet) {
        throw std::runtime_error("downloader::downloader - Failed to initialize WinHTTP");
    }
}

std::vector<char> downloader::download_file(const std::string &url) {
    // Split the URL
    const size_t protocol_end = url.find("://");
    if (protocol_end == std::string::npos) {
        throw std::runtime_error("downloader::download_file - Invalid URL: " + url + " (missing protocol)");
    }

    const std::string protocol = url.substr(0, protocol_end);
    if (protocol != "http" && protocol != "https") {
        throw std::runtime_error("downloader::download_file - Unsupported protocol: " + protocol);
    }

    const size_t host_start = protocol_end + 3;
    const size_t path_start = url.find('/', host_start);
    const std::string host = url.substr(host_start, path_start - host_start);
    const std::string path = url.substr(path_start);

    const auto connection = WinHttpConnect(
        _internet,
        std::wstring(host.begin(), host.end()).c_str(),
        protocol == "https" ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT,
        0
    );

    if (!connection) {
        throw std::runtime_error("downloader::download_file - Failed to connect to host: " + host);
    }

    const auto request = WinHttpOpenRequest(
        connection,
        L"GET",
        std::wstring(path.begin(), path.end()).c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        protocol == "https" ? WINHTTP_FLAG_SECURE : 0
    );

    if (!request) {
        WinHttpCloseHandle(connection);
        throw std::runtime_error("downloader::download_file - Failed to open request for URL: " + url);
    }

    if (!WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        throw std::runtime_error("downloader::download_file - Failed to send request to URL: " + url);
    }

    if (!WinHttpReceiveResponse(request, nullptr)) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        throw std::runtime_error("downloader::download_file - Failed to receive response from URL: " + url);
    }

    DWORD total_read = 0;
    std::vector<char> data;

    while (true) {
        char buffer[8192];
        DWORD bytes_read = 0;
        if (!WinHttpReadData(request, buffer, sizeof(buffer), &bytes_read)) {
            WinHttpCloseHandle(request);
            WinHttpCloseHandle(connection);
            throw std::runtime_error("downloader::download_file - Failed to read data from URL: " + url);
        }

        if (bytes_read == 0) {
            break; // End of data
        }

        data.insert(data.end(), buffer, buffer + bytes_read);
        total_read += bytes_read;
    }

    // Clean up
    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);

    return data;
}
