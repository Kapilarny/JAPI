#include "downloader.h"

#include <windows.h>
#include <winhttp.h>

Version GetCurrentJAPIVersion() {
    
}

std::vector<uint8_t> DownloadFile(std::string url) {
    std::vector<uint8_t> buffer;

    HINTERNET hSession = WinHttpOpen(L"ASBR Updater/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if(!hSession) {
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
        WinHttpCloseHandle(hSession);
        return buffer;
    }

    // Get the url path
    std::string::iterator path_start = url_end;
    std::string::iterator path_end = url.end();

    std::wstring path(path_start, path_end);

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if(!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return buffer;
    }

    if(!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return buffer;
    }

    if(!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return buffer;
    }

    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    do {
        dwSize = 0;
        if(!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return buffer;
        }

        if(!dwSize) {
            break;
        }

        buffer.resize(buffer.size() + dwSize);
        if(!WinHttpReadData(hRequest, buffer.data() + dwDownloaded, dwSize, &dwDownloaded)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return buffer;
        }
    } while(dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return buffer;
}