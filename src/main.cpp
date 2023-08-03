#include <Windows.h>

#include "japi.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            AllocConsole();
            SetConsoleTitle("JojoAPI Console");
            FILE* file = nullptr;
            freopen_s(&file, "CONOUT$", "w", stdout);
            freopen_s(&file, "CONIN$", "r", stdin);

            const HANDLE thread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE) JAPI::Init, hinstDLL, 0, nullptr);
            if (thread) {
                CloseHandle(thread);
            }
            
            break;
    }
    return TRUE;
}
