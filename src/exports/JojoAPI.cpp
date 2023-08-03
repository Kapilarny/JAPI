#include "JojoAPI.h"

#include "japi.h"
#include "utils/logger.h"
#include "utils/mem.h"
#include "utils/hooks.h"

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
    LOG_FATAL("", message);
}

void JAPI_LogError(std::string message) {
    LOG_ERROR("", message);
}

void JAPI_LogWarn(std::string message) {
    LOG_WARN("", message);
}

void JAPI_LogInfo(std::string message) {
    LOG_INFO("", message);
}

void JAPI_LogDebug(std::string message) {
    LOG_DEBUG("", message);
}

void JAPI_LogTrace(std::string message) {
    LOG_TRACE("", message);
}