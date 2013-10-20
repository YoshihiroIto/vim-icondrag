//=============================================================================
// FILE: dllmain.cpp
// AUTHOR: Yoshihiro Ito <yo.i.jewelry.bab@gmail.com@gmail.com>
// License: MIT license
//     Permission is hereby granted, free of charge, to any person obtaining
//     a copy of this software and associated documentation files (the
//     "Software"), to deal in the Software without restriction, including
//     without limitation the rights to use, copy, modify, merge, publish,
//     distribute, sublicense, and/or sell copies of the Software, and to
//     permit persons to whom the Software is furnished to do so, subject to
//     the following conditions:
//
//     The above copyright notice and this permission notice shall be included
//     in all copies or substantial portions of the Software.
//
//     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//     OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//     MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//     IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//     CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//     TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//     SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//=============================================================================
#include "stdafx.h"
#include "Core.h"

namespace
{
HMODULE gvimModule  = 0;     // Gvim本体
HMODULE selfHandle  = 0;     // 常駐用
HWND    gvimHwnd    = 0;
WNDPROC oldWndProc  = 0;
Core    core;

LRESULT CALLBACK IconDragWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case Core::WM_GETDATA:{ if (core.OnGETDATA(      wParam, lParam)) return 0; } break;
        case WM_NCLBUTTONDOWN:{ if (core.OnNCLBUTTONDOWN(wParam, lParam)) return 0; } break;
        case WM_NCRBUTTONDOWN:{ if (core.OnNCRBUTTONDOWN(wParam, lParam)) return 0; } break;
        case WM_MOUSEMOVE:{     if (core.OnMOUSEMOVE(    wParam, lParam)) return 0; } break;
        case WM_LBUTTONUP:{     if (core.OnLBUTTONUP(    wParam, lParam)) return 0; } break;
        case WM_RBUTTONUP:{     if (core.OnRBUTTONUP(    wParam, lParam)) return 0; } break;
        case WM_TIMER:{         if (core.OnTIMER(        wParam, lParam)) return 0; } break;
    }

    return CallWindowProc(oldWndProc, hWnd, uMsg, wParam, lParam);
}

void Initialize(const char *args)
{
    if (selfHandle == 0)
    {
        gvimHwnd = (HWND)args;

        OleInitialize(NULL);

        // サブクラス化
        #define GWL_WNDPROC (-4)
        oldWndProc = (WNDPROC)GetWindowLongPtr(gvimHwnd, GWL_WNDPROC);
        SetWindowLongPtr(gvimHwnd, GWL_WNDPROC, (LONG_PTR)IconDragWndProc);

        // 常駐
        char selfPath[MAX_PATH];
        GetModuleFileNameA(gvimModule, selfPath, sizeof(selfPath));
        selfHandle = LoadLibraryA(selfPath);
        core.Initialize(gvimHwnd);
    }
}

void Finalize()
{
    if (selfHandle != 0)
    {
        core.Finalize();

        // 常駐解除
        FreeLibrary(selfHandle);
        selfHandle = 0;

        // サブクラス化を戻す
        SetWindowLongPtr(gvimHwnd, GWLP_WNDPROC, (LONG_PTR)oldWndProc);

        OleUninitialize();
    }
}

}

#define EXPORT extern "C" __declspec(dllexport)

EXPORT char *IconDrag_Enable(const char *args)
{
    Initialize(args);

    return NULL;
}

EXPORT char *IconDrag_Disable(const char *args)
{
    Finalize();

    return NULL;
}

EXPORT char *IconDrag_SetCurrentFilepath(const char *args)
{
    core.SetFilepath(args == NULL ? "" : args);

    return NULL;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        gvimModule = hModule;
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        Finalize();
        break;
    }
    return TRUE;
}

