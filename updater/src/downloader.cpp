#include "downloader.h"

#include <filesystem>
#include <windows.h>
#include <winhttp.h>

#include "logger.h"

void DownloadUpdater(Version version) {
    std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/files/updater/" + VersionString(version) + ".dll");

    if(buffer.empty()) {
        JERROR("Failed to download the latest updater! Is the internet down?");
        return;
    }

    // Overwrite the old updater
    std::ofstream out("JAPIUpdaterLib.dll", std::ios::binary | std::ios::out | std::ios::trunc);
    out.write((char*)buffer.data(), buffer.size());
    out.close();

    JINFO("Downloaded the latest updater!");
}

Version GetLatestUpdaterVersion() {
    std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/master/updater_version.txt");

    if(buffer.empty()) {
        JERROR("Failed to download get the latest updater version! Is the internet down?");

        return {0, 0, 0};
    }

    std::string version_str(buffer.begin(), buffer.end());

    JINFO("Latest updater version: " + version_str);

    return ParseVersion(version_str);
}

bool Is404(std::vector<uint8_t>& buffer) {
    std::string str(buffer.begin(), buffer.end());

    return str.find("404: Not Found") != std::string::npos;
}

std::vector<uint8_t> DownloadFile(std::string url) {
    std::vector<uint8_t> buffer;

    JINFO("Downloading " + url);

    HINTERNET hSession = WinHttpOpen(L"ASBR Updater/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if(!hSession) {
        JERROR("Failed to open WinHTTP session!\n");
        return buffer;
    }

    // Get the url hostname
    std::string::iterator url_start = url.begin();
    std::string::iterator url_end = url.begin();
    for(; url_end != url.end(); url_end++) {
        if(*url_end == '/') {
            break;
        }
    }

    std::wstring hostname(url_start, url_end);

    HINTERNET hConnect = WinHttpConnect(hSession, hostname.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if(!hConnect) {
        JERROR("Failed to connect to the host!");

        WinHttpCloseHandle(hSession);
        return buffer;
    }

    // Get the url path
    std::string::iterator path_start = url_end;
    std::string::iterator path_end = url.end();

    std::wstring path(path_start, path_end);

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if(!hRequest) {
        JERROR("Failed to open the request!");

        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return buffer;
    }

    if(!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        JERROR("Failed to send the request!");

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return buffer;
    }

    if(!WinHttpReceiveResponse(hRequest, NULL)) {
        JERROR("Failed to receive the response!");

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return buffer;
    }

    uint64_t totalDownloaded = 0;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    do {
        dwSize = 0;
        if(!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            JERROR("Failed to query data!");

            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return buffer;
        }

        if(!dwSize) {
            break;
        }

        buffer.resize(buffer.size() + dwSize);
        if(!WinHttpReadData(hRequest, buffer.data() + totalDownloaded, dwSize, &dwDownloaded)) {
            JERROR("Failed to read data!");

            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return buffer;
        }

        totalDownloaded += dwDownloaded;
    } while(dwSize > 0);

    JINFO("Downloaded " + std::to_string(buffer.size()) + " bytes");

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if(Is404(buffer)) {
        JERROR("404: Not Found\n");
        buffer.clear();
    }

    return buffer;
}