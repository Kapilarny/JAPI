#include "JojoAPI.h"

#include "japi.h"
#include "utils/logger.h"
#include "utils/mem.h"
#include "utils/hooks.h"

const char* GetModGUID(void* retAddr) {
    HMODULE hModule = NULL;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)retAddr, &hModule);

    // Get the mod info
    auto getModInfo = (ModMeta (*)()) GetProcAddress(hModule, "GetModInfo");
    if(!getModInfo) {
        return "Unknown";
    }
    
    return getModInfo().guid;
}

void JAPI_PatchASBRMem(void* address, void* data, size_t size) {
    PatchEx((BYTE*)address, (BYTE*)data, size, GetCurrentProcess());
}

void JAPI_PatchMem(void* address, void* data, size_t size, HANDLE processHandle) {
    PatchEx((BYTE*)address, (BYTE*)data, size, processHandle);
}

bool JAPI_HookFunction(Hook* hook) {
    return HookFunction(hook);
}

bool JAPI_UnhookFunction(Hook* hook) {
    return UnhookFunction(hook);
}

void JAPI_LogFatal(std::string message) {
    LOG_FATAL(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), message);
}

void JAPI_LogError(std::string message) {
    LOG_ERROR(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), message);
}

void JAPI_LogWarn(std::string message) {
    LOG_WARN(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), message);
}

void JAPI_LogInfo(std::string message) {
    LOG_INFO(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), message);
}

void JAPI_LogDebug(std::string message) {
    LOG_DEBUG(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), message);
}

void JAPI_LogTrace(std::string message) {
    LOG_TRACE(GetModGUID(__builtin_extract_return_addr(__builtin_return_address(0))), message);
}