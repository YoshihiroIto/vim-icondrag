// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

namespace
{
HMODULE gvimModule = 0;     // Gvim本体
HMODULE selfHandle = 0;     // 常駐用

void Initialize()
{
    if (selfHandle == 0)
    {
        char selfPath[MAX_PATH];
        GetModuleFileNameA(gvimModule, selfPath, sizeof(selfPath));

        selfHandle = LoadLibraryA(selfPath);
    }
}

void Finalize()
{
    if (selfHandle != 0)
    {
        FreeLibrary(selfHandle);
        selfHandle = 0;
    }
}

}

#define EXPORT extern "C" __declspec(dllexport)

EXPORT void *IconDrag_Enable(const char *args)
{
    MessageBoxA((HWND)args, "IconDrag_Enable", "IconDrag", MB_OK);
    Initialize();

    return NULL;
}

EXPORT void * IconDrag_Disable(const char *args)
{
    MessageBoxA((HWND)args, "IconDrag_Disable", "IconDrag", MB_OK);
    Finalize();

    return NULL;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBoxA(0, "DLL_PROCESS_ATTACH", "IconDrag", MB_OK);

        gvimModule = hModule;
        break;

    case DLL_THREAD_ATTACH:
        //MessageBoxA(0, "DLL_THREAD_ATTACH", "IconDrag", MB_OK);
        break;

    case DLL_THREAD_DETACH:
        //MessageBoxA(0, "DLL_THREAD_DETACH", "IconDrag", MB_OK);
        break;

    case DLL_PROCESS_DETACH:
        MessageBoxA(0, "DLL_PROCESS_DETACH", "IconDrag", MB_OK);
        Finalize();
        break;
    }
    return TRUE;
}
