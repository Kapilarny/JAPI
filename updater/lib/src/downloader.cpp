#include "downloader.h"

#include <filesystem>
#include <windows.h>
#include <winhttp.h>

#include "logger.h"
#include "game_type.h"

Version GetLatestJAPIVersion() {
    std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/master/version.txt");

    if(buffer.empty()) {
        JERROR("Failed to download get the latest JAPI version! Is the internet down?");
        return {0, 0, 0};
    }

    std::string version_str(buffer.begin(), buffer.end());

    JINFO("Latest JAPI version: " + version_str);

    return ParseVersion(version_str);
}

std::vector<std::string> GetLatestJAPIDlls() {
    std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/master/dlls.txt");

    if(buffer.empty()) {
        JERROR("Failed to download the latest JAPI dlls! Is the internet down?");
        return {};
    }

    std::string dlls_str(buffer.begin(), buffer.end());

    // Split the string by newlines
    std::vector<std::string> dlls;
    std::string::iterator it = dlls_str.begin();
    std::string::iterator end = dlls_str.end();

    std::string dll;
    for(; it != end; it++) {
        if(*it == '\n') {
            dlls.push_back(dll);
            dll.clear();
            continue;
        }

        dll += *it;
    }

    if(!dll.empty()) dlls.push_back(dll);

    return dlls;
}

bool Is404(std::vector<uint8_t>& buffer) {
    return buffer.size() == 15 && strcmp((const char*)buffer.data(), "404: Not Found") == 0;
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

uint16_t DownloadASBR() {
    // Get the hash
    std::ifstream asbr_orig("ASBR.exe");
    uint16_t hash = ComputeCRC16Hash(asbr_orig);

    JTRACE("Checking if ASBR.exe is already unpacked...");

    if(IsUnpackedHash(hash)) {
        JINFO("ASBR.exe is already unpacked! Skipping download...");
        return hash;
    }

    std::string str_hash = std::to_string(hash);

    asbr_orig.close();

    JTRACE("ASBR.exe is not unpacked! Downloading the new executable...");
    JTRACE("Hash: " + str_hash);

    // 27687

    JTRACE("Renamed the original ASBR.exe to ASBR_orig.exe");

    // Grab the file
    std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/files/asbr/" + str_hash);
    // std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/files/unpacked_hashes.txt");

    if(buffer.empty()) {
        JFATAL("Failed to download ASBR! Is this hash correct? (" + str_hash + ") ABORTING");

        exit(1);

        return -1;
    }

    // Rename the original ASBR.exe
    if(std::filesystem::exists("ASBR.exe")) {
        std::filesystem::rename("ASBR.exe", "ASBR_orig.exe");
    }

    // Overwrite the current ASBR.exe
    std::ofstream asbr("ASBR.exe", std::ios::out | std::ios::trunc | std::ios::binary);
    asbr.write((char*)buffer.data(), buffer.size());
    asbr.close();

    // Create a new hash
    std::ifstream asbr_file("ASBR.exe");
    if(!asbr_file.good()) {
        JFATAL("Failed to open the new ASBR.exe to calculate the hash! ABORTING\n");

        exit(1);

        return -1;
    }

    uint16_t new_hash = ComputeCRC16Hash(asbr_file);

    JINFO("ASBR.exe updated!");

    return new_hash;
}

void DownloadAdditionalDLLs() {
    std::vector<std::string> dlls = GetLatestJAPIDlls();

    for(std::string dll : dlls) {
        std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/files/dlls/" + dll);

        if(buffer.empty()) {
            JFATAL("Failed to download " + dll + "! No internet?");
            continue;
        }

        // Overwrite the current dll
        std::ofstream dll_file(dll, std::ios::out | std::ios::trunc | std::ios::binary);
        dll_file.write((char*)buffer.data(), buffer.size());
        dll_file.close();

        JINFO(dll + " updated!");
    }
}

void DownloadJAPI(Version version) {
    // If d3dcompiler_47_o.dll doesnt exists, rename d3dcompiler_47.dll to it
    if(!std::filesystem::exists("d3dcompiler_47_o.dll")) {
        std::filesystem::rename("d3dcompiler_47.dll", "d3dcompiler_47_o.dll");
    }

    // Grab the file
    std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/files/japi/" + VersionString(version) + ".dll");

    if(buffer.empty()) {
        JFATAL("Failed to download JAPI! Is this version correct? (" + VersionString(version) + ")");
        return;
    }

    // Overwrite the current d3dcompiler_47.dll
    std::ofstream d3dcompiler_47("d3dcompiler_47.dll", std::ios::out | std::ios::trunc | std::ios::binary);
    d3dcompiler_47.write((char*)buffer.data(), buffer.size());
    d3dcompiler_47.close();

    JINFO("JAPI updated!");
}

void CreateSteamAppID() {
    // Create steam_appid.txt
    std::ofstream steam_appid("steam_appid.txt", std::ios::out | std::ios::trunc);
    steam_appid << GetGameData().steam_appid;
    steam_appid.close();

    JINFO("Created steam_appid.txt");
}

bool IsUnpackedHash(uint16_t hash) {
    std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/files/unpacked_hashes.txt");

    if(buffer.empty()) {
        JERROR("Failed to download the unpacked hashes! Is the internet down?");
        return false;
    }

    std::string hashes_str(buffer.begin(), buffer.end());

    // Split the string by newlines
    std::vector<std::string> hashes;
    std::string::iterator it = hashes_str.begin();
    std::string::iterator end = hashes_str.end();

    std::string hash_str;
    for(; it != end; it++) {
        if(*it == '\n') {
            hashes.push_back(hash_str);
            hash_str.clear();
            continue;
        }

        hash_str += *it;
    }

    if(!hash_str.empty()) {
        hashes.push_back(hash_str);
    }

    for(std::string& str : hashes) {
        JTRACE("Checking hash: " + str);
        if(str == std::to_string(hash)) {
            return true;
        }
    }

    return false;
}