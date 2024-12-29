//
// Created by user on 28.12.2024.
//

#include "downloader.h"

#include <filesystem>
#include <windows.h>
#include <winhttp.h>

#include "logger.h"

bool downloader::is_404(std::vector<uint8_t>& buffer) {
    return buffer.size() == 15; // This is so fucking stupid i love this
}

std::vector<uint8_t> downloader::download_file(std::string url) {
    std::vector<uint8_t> buffer;

    JTRACE("Downloading " + url);

    //  URL_COMPONENTS urlComp;
    URL_COMPONENTS urlComp;
    ZeroMemory(&urlComp, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);

    wchar_t hostName[256];
    wchar_t urlPath[2048];

    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = sizeof(hostName) / sizeof(wchar_t);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(wchar_t);

    // Crack the URL
    if(!WinHttpCrackUrl(std::wstring(url.begin(), url.end()).c_str(), url.size(), 0, &urlComp)) {
        JERROR("Failed to crack the URL!");

        return buffer;
    }

    HINTERNET hSession = WinHttpOpen(L"ASBR Updater/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if(!hSession) {
        JERROR("Failed to open WinHTTP session!\n");
        return buffer;
    }

    // Connect to the server
    HINTERNET hConnect = WinHttpConnect(hSession, hostName, urlComp.nPort, 0);
    if(!hConnect) {
        JERROR("Failed to connect to the host!");

        WinHttpCloseHandle(hSession);
        return buffer;
    }

    // Open the request
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, urlComp.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);
    if(!hRequest) {
        JERROR("Failed to open the request!");

        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return buffer;
    }

    // Send the request
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        JERROR("Failed to send the request!");

        DWORD error = GetLastError();
        JERROR("Error code: %d", error);

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        return buffer;
    }

    // Receive the response
    if (!WinHttpReceiveResponse(hRequest, NULL)) {
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

    JTRACE("Downloaded " + std::to_string(buffer.size()) + " bytes");

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if(is_404(buffer)) {
        JERROR("404: Not Found\n");
        buffer.clear();
    }

    return buffer;
}
