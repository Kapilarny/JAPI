// -----------------------------------------------------------------------------
// WARNING: This file is part of JAPI's internal implementation.
//          Do NOT use any of the functions or definitions declared here in your plugins,
//          as they are subject to change without notice and may break your plugins.
// -----------------------------------------------------------------------------

#ifndef JAPI_INTERNAL_JAPI_H
#define JAPI_INTERNAL_JAPI_H

#include <windows.h>

#ifndef JEXP
#define JEXP extern "C" __declspec(dllexport)
#endif

// This function is called by the preload dll to initialize the main JAPI instance
// DO NOT CALL THIS FUNCTION IN YOUR PLUGINS!
JEXP void __stdcall internal_japi_initialize(HMODULE h_module);

#endif //JAPI_INTERNAL_JAPI_H