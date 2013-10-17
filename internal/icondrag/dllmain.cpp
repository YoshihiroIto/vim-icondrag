#include "stdafx.h"
#include "Core.h"

namespace
{

HMODULE gvimModule  = 0;     // Gvimñ{ëÃ
HMODULE selfHandle  = 0;     // èÌíìóp
HHOOK   hook        = 0;
Core    core;

LRESULT HookProc( int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
	{
	    if (nCode == HC_ACTION)
		{
		    const CWPSTRUCT *pcwp = (CWPSTRUCT *)lParam;

 char str[256];

            if (pcwp->message == WM_CLOSE) {
                MessageBeep(MB_OK);
                sprintf(str,
                    "Window %X Ç™ï¬Ç∂ÇÁÇÍÇÊÇ§Ç∆ÇµÇƒÇ¢Ç‹Ç∑",
                    pcwp->hwnd);
                MessageBoxA(pcwp->hwnd, str, "OK", MB_OK);
            }


			switch(pcwp->message)
			{
				case Core::WM_GETDATA:{	if (core.OnGETDATA(		    pcwp->wParam, pcwp->lParam))    return 1;	}   break;
				case WM_NCLBUTTONDOWN:{	if (core.OnNCLBUTTONDOWN(	pcwp->wParam, pcwp->lParam))    return 1;	}   break;
				case WM_NCRBUTTONDOWN:{	if (core.OnNCRBUTTONDOWN(	pcwp->wParam, pcwp->lParam))    return 1;	}   break;
				case WM_MOUSEMOVE:{		if (core.OnMOUSEMOVE(		pcwp->wParam, pcwp->lParam))    return 1;	}   break;
				case WM_LBUTTONUP:{		if (core.OnLBUTTONUP(		pcwp->wParam, pcwp->lParam))    return 1;	}   break;
				case WM_RBUTTONUP:{		if (core.OnRBUTTONUP(		pcwp->wParam, pcwp->lParam))    return 1;	}   break;
				case WM_TIMER:{			if (core.OnTIMER(			pcwp->wParam, pcwp->lParam))    return 1;	}   break;
		    }
		}
	}

    return CallNextHookEx(hook, nCode, wParam, lParam);
}

void Initialize(const char *args)
{
    if (selfHandle == 0)
    {
        char selfPath[MAX_PATH];
        GetModuleFileNameA(gvimModule, selfPath, sizeof(selfPath));

	    #define GWL_HINSTANCE       (-6)
	    HINSTANCE hinst = (HINSTANCE)GetWindowLong((HWND)args, GWL_HINSTANCE);
        hook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)HookProc, hinst, GetCurrentThreadId());

        // èÌíì
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

        // èÌíìâèú
        FreeLibrary(selfHandle);
        selfHandle = 0;
    }
}

}

#define EXPORT extern "C" __declspec(dllexport)

EXPORT void *IconDrag_Enable(const char *args)
{
    MessageBoxA((HWND)args, "IconDrag_Enable", "IconDrag", MB_OK);
    Initialize(args);

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
