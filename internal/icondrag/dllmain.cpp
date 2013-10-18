#include "stdafx.h"
#include "Core.h"

namespace
{
HMODULE gvimModule  = 0;     // Gvim�{��
HMODULE selfHandle  = 0;     // �풓�p
HHOOK   hook        = 0;
WNDPROC oldWndProc = NULL;
Core    core;

LRESULT CALLBACK MenuWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case Core::WM_GETDATA:{	if (core.OnGETDATA(		    wParam, lParam))    return 0;	}   break;
		case WM_NCLBUTTONDOWN:{	if (core.OnNCLBUTTONDOWN(	wParam, lParam))    return 0;	}   break;
		case WM_NCRBUTTONDOWN:{	if (core.OnNCRBUTTONDOWN(	wParam, lParam))    return 0;	}   break;
		case WM_MOUSEMOVE:{		if (core.OnMOUSEMOVE(		wParam, lParam))    return 0;	}   break;
		case WM_LBUTTONUP:{		if (core.OnLBUTTONUP(		wParam, lParam))    return 0;	}   break;
		case WM_RBUTTONUP:{		if (core.OnRBUTTONUP(		wParam, lParam))    return 0;	}   break;
		case WM_TIMER:{			if (core.OnTIMER(			wParam, lParam))    return 0;	}   break;
	}

	return CallWindowProc(oldWndProc, hWnd, uMsg, wParam, lParam);
}

void Initialize(const char *args)
{
    if (selfHandle == 0)
    {
		OleInitialize(NULL);

		// �T�u�N���X��
		#define GWL_WNDPROC (-4)
		oldWndProc = (WNDPROC)GetWindowLongPtr((HWND)args, GWL_WNDPROC);
        SetWindowLongPtr((HWND)args, GWL_WNDPROC, (LONG_PTR)MenuWndProc);

        // �풓
		char selfPath[MAX_PATH];
        GetModuleFileNameA(gvimModule, selfPath, sizeof(selfPath));
        selfHandle = LoadLibraryA(selfPath);
		core.Initialize((HWND)args);
    }
}

void Finalize()
{
    if (selfHandle != 0)
    {
		core.Finalize();

        UnhookWindowsHookEx(hook);
        hook = 0;

        // �풓����
        FreeLibrary(selfHandle);
        selfHandle = 0;

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
