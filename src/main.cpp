#include <Windows.h>

#include "japi.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            const HANDLE thread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE) JAPI::Init, hinstDLL, 0, nullptr);
            if (thread) {
                CloseHandle(thread);
            }
            
            break;
    }
    return TRUE;
}
