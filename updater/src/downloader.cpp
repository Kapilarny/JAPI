#include "downloader.h"

#include <filesystem>
#include <windows.h>
#include <winhttp.h>

Version GetLatestJAPIVersion() {
    std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/master/version.txt");

    if(buffer.empty()) {
        std::cout << "Failed to download the latest JAPI version! Is the internet down?\n";
        return {0, 0, 0};
    }

    std::string version_str(buffer.begin(), buffer.end());

    std::cout << "Latest JAPI version: " + version_str + "\n";

    return ParseVersion(version_str);
}

std::vector<std::string> GetLatestJAPIDlls() {
    std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/master/dlls.txt");

    if(buffer.empty()) {
        std::cout << "Failed to download the latest JAPI dlls! Is the internet down?\n";
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

    return dlls;
}

bool Is404(std::vector<uint8_t>& buffer) {
    std::string str(buffer.begin(), buffer.end());

    return str.find("404: Not Found") != std::string::npos;
}

std::vector<uint8_t> DownloadFile(std::string url) {
    std::vector<uint8_t> buffer;

    std::cout << "Downloading " + url + "\n";

    HINTERNET hSession = WinHttpOpen(L"ASBR Updater/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if(!hSession) {
        std::cout << "Failed to open WinHTTP session!\n";
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

    std::cout << "Hostname: " + std::string(hostname.begin(), hostname.end()) + "\n";

    HINTERNET hConnect = WinHttpConnect(hSession, hostname.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if(!hConnect) {
        std::cout << "Failed to connect to the host!\n";

        WinHttpCloseHandle(hSession);
        return buffer;
    }

    // Get the url path
    std::string::iterator path_start = url_end;
    std::string::iterator path_end = url.end();

    std::wstring path(path_start, path_end);

    std::cout << "Path: " + std::string(path.begin(), path.end()) + "\n";

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if(!hRequest) {
        std::cout << "Failed to open the request!\n";

        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return buffer;
    }

    if(!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        std::cout << "Failed to send the request!\n";

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return buffer;
    }

    if(!WinHttpReceiveResponse(hRequest, NULL)) {
        std::cout << "Failed to receive the response!\n";

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
            std::cout << "Failed to query data!\n";

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
            std::cout << "Failed to read data!\n";

            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return buffer;
        }

        totalDownloaded += dwDownloaded;
    } while(dwSize > 0);

    std::cout << "Downloaded " + std::to_string(buffer.size()) + " bytes\n";

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if(Is404(buffer)) {
        std::cout << "404: Not Found\n";
        buffer.clear();
    }

    return buffer;
}

uint16_t DownloadASBR() {
    // Get the hash
    std::ifstream asbr_orig("ASBR.exe");
    uint16_t hash = ComputeCRC16Hash(asbr_orig);
    std::string str_hash = std::to_string(hash);

    asbr_orig.close();

    // Rename the original ASBR.exe
    if(std::filesystem::exists("ASBR.exe")) {
        std::filesystem::rename("ASBR.exe", "ASBR_orig.exe");
    }

    // Grab the file
    std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/files/asbr/" + str_hash + ".exe");

    if(buffer.empty()) {
        std::cout << "Failed to download ASBR! Is this hash correct? (" + str_hash + ")\n";
        return -1;
    }

    // Overwrite the current ASBR.exe
    std::ofstream asbr("ASBR.exe", std::ios::out | std::ios::trunc | std::ios::binary);
    asbr.write((char*)buffer.data(), buffer.size());
    asbr.close();

    std::cout << "ASBR.exe updated!\n";

    return hash;
}

void DownloadAdditionalDLLs() {
    std::vector<std::string> dlls = GetLatestJAPIDlls();

    for(std::string dll : dlls) {
        std::vector<uint8_t> buffer = DownloadFile("raw.githubusercontent.com/Kapilarny/JAPI/files/dlls/" + dll);

        if(buffer.empty()) {
            std::cout << "Failed to download " + dll + "! Is this correct?\n";
            continue;
        }

        // Overwrite the current dll
        std::ofstream dll_file(dll, std::ios::out | std::ios::trunc | std::ios::binary);
        dll_file.write((char*)buffer.data(), buffer.size());
        dll_file.close();

        std::cout << dll + " updated!\n";
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
        std::cout << "Failed to download JAPI! Is this version correct? (" + VersionString(version) + ")\n";
        return;
    }

    // Overwrite the current d3dcompiler_47.dll
    std::ofstream d3dcompiler_47("d3dcompiler_47.dll", std::ios::out | std::ios::trunc | std::ios::binary);
    d3dcompiler_47.write((char*)buffer.data(), buffer.size());
    d3dcompiler_47.close();

    std::cout << "JAPI updated!\n";
}

uint16_t InstallASBRAndGetHash() {
    DownloadASBR();

    std::ifstream asbr_file("ASBR.exe");
    if(!asbr_file.good()) {
        std::cout << "Failed to open the new ASBR.exe to calculate the hash! ABORTING\n";
        return 1;
    }

    return ComputeCRC16Hash(asbr_file);
}

void RemoveEAC() {
    // Remove the EAC

    // EasyAntiCheat_EOS_Setup.exe uninstall 3930c3bc57134e19ab46e07c967aa013
    // Run the uninstaller
    system("EasyAntiCheat/EasyAntiCheat_EOS_Setup.exe uninstall 3930c3bc57134e19ab46e07c967aa013");

    // Wait 3 seconds for good measure
    Sleep(3000);

    // Create steam_appid.txt
    std::ofstream steam_appid("steam_appid.txt", std::ios::out | std::ios::trunc);
    steam_appid << "1372110";
    steam_appid.close();

    std::cout << "EAC removed!\n";
}