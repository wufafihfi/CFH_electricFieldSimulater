#pragma once

static void CheckDllLoaded(const char* dllName)
{
    HMODULE h = GetModuleHandleA(dllName);
    if (h != nullptr)
    {
        char path[MAX_PATH] = { 0 };
        GetModuleFileNameA(h, path, MAX_PATH);

        LOG_INFO_STREAM << u8"[DLL] ря╪сть: " << dllName << " -> " << path;
    }
    else
    {
        LOG_INFO_STREAM << u8"[DLL] н╢╪сть: " << dllName;
    }
}

void CheckRuntimeDlls()
{
    // x86 CRT DLL
    LOG_INFO(u8"===x86 CRT DLL===");
    CheckDllLoaded("msvcp140.dll");
    CheckDllLoaded("msvcp140_1.dll");
    CheckDllLoaded("msvcp140_2.dll");
    CheckDllLoaded("msvcp140_atomic_wait.dll");
    CheckDllLoaded("msvcp140_codecvt_ids.dll");
    CheckDllLoaded("vcruntime140.dll");
    CheckDllLoaded("concrt140.dll");
    
    // other
    LOG_INFO(u8"===Other DLL===");
    CheckDllLoaded("sfml-graphics-3.dll");
    CheckDllLoaded("sfml-window-3.dll");
    CheckDllLoaded("sfml-system-3.dll");
    CheckDllLoaded("openal32.dll");
    CheckDllLoaded("freetype.dll");
    CheckDllLoaded("zlib1.dll");
    CheckDllLoaded("libpng16.dll");
    CheckDllLoaded("bz2.dll");
    CheckDllLoaded("brotlidec.dll");
    CheckDllLoaded("brotlicommon.dll");
}