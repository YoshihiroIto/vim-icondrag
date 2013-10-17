#include "StdAfx.h"
#include "Core.h"

extern "C"{
#include "OleDragDrop.h"
}

enum
{
    WM_GETDATA			= 1977,
    WM_TIMER_SYSMENU,
};

// --------------------------------------------------------------------------
void Core::Initialize(HWND hwnd)
{
    dragging				= false;
    timerState				= 0;
    isActiveSysmenuTimer	= false;
    isLeftClick			    = false;
    drawStartXpos			= 0;
    drawStartYpos			= 0;
    this->hwnd = hwnd;

	// test
	filename = L"C:\\Users\\yh-ito\\Downloads\\test.txt";
}
// --------------------------------------------------------------------------
void Core::Finalize()
{
    KillSysMenuTimer();
}
// --------------------------------------------------------------------------
bool Core::OnGETDATA(WPARAM wParam, LPARAM lParam)
{
    switch(wParam)
	{
        case CF_HDROP:
	    {
            const DWORD	buffer_size	= sizeof(DROPFILES) + (MAX_PATH * 2) + 1 + 1;
            //
            HDROP	drop_handle	= (HDROP)GlobalAlloc(GHND | GMEM_SHARE, buffer_size);
            {
                DROPFILES *	dropfiles	= (DROPFILES *)GlobalLock(drop_handle);

                ZeroMemory(dropfiles, buffer_size);
                dropfiles->pFiles	= sizeof(DROPFILES);		// ファイル名のリストまでのオフセット
                dropfiles->pt.x		= 0;
                dropfiles->pt.y		= 0;
                dropfiles->fNC		= false;
                dropfiles->fWide	= TRUE;				    // ワイドキャラの場合は TRUE
                CopyMemory(dropfiles + 1, filename.c_str(), filename.size() * 2);
            }

            //
            GlobalUnlock(drop_handle);
            *((HANDLE *)lParam)	= drop_handle;

            break;
        }
    }

    return true;
}

// --------------------------------------------------------------------------
bool Core::OnNCBUTTONDOWN_Core(WPARAM wParam, LPARAM lParam)
{
    int	iFileNameLength	= (int)filename.size();

    if(	(wParam == HTSYSMENU) &&
        (iFileNameLength > 0)){
        dragging		= true;
        timerState		= 0;
        drawStartXpos	= (int)LOWORD(lParam);
        drawStartYpos	= (int)HIWORD(lParam);

        //
        SetSysMenuTimer();

        //
        SetCapture(hwnd);

        return true;
    }
    else
    {
        return false;
    }
}
// --------------------------------------------------------------------------
bool Core::OnNCLBUTTONDOWN(WPARAM wParam, LPARAM lParam)
{
	MessageBoxA(hwnd, "OnNCLBUTTONDOWN", "IconDrag", MB_OK);

    isLeftClick	= true;				// 左クリック

    return OnNCBUTTONDOWN_Core(wParam, lParam);
}
// --------------------------------------------------------------------------
bool Core::OnNCRBUTTONDOWN(WPARAM wParam, LPARAM lParam)
{
	MessageBoxA(hwnd, "OnNCRBUTTONDOWN", "IconDrag", MB_OK);

    isLeftClick	= false;			// 右クリック

    return OnNCBUTTONDOWN_Core(wParam, lParam);
}
// --------------------------------------------------------------------------
bool Core::OnMOUSEMOVE(WPARAM wParam, LPARAM lParam)
{
	MessageBoxA(hwnd, "OnMOUSEMOVE", "IconDrag", MB_OK);

    if(dragging && (IsInDoubleClickRect() == false))
	{
        KillSysMenuTimer();

        // ドラッグ開始
        UINT	cf[]	= {CF_HDROP};
        int		iEffect	= (isLeftClick == true) ? DROPEFFECT_COPY : DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;

		OLE_IDropSource_Start(hwnd, (UINT)WM_GETDATA, cf, sizeof(cf) / sizeof(cf[0]), iEffect);

        // ドラッグ終了
        dragging	= false;
        ReleaseCapture();

        return true;
    }
	else
	{
        return false;
    }
}
// --------------------------------------------------------------------------
bool Core::IsInDoubleClickRect()
{
    #define	ABS(A)	(((A) >= 0) ? (A) : -(A))

    POINT	cursor_pos;
    GetCursorPos(&cursor_pos);

    return
        (ABS(cursor_pos.x - drawStartXpos) < (GetSystemMetrics(SM_CXDOUBLECLK) >> 1)) &&
        (ABS(cursor_pos.y - drawStartYpos) < (GetSystemMetrics(SM_CYDOUBLECLK) >> 1));

    #undef ABS
}

// --------------------------------------------------------------------------
bool Core::OnBUTTONUP_Core(WPARAM wParam, LPARAM lParam){

    // ドラッグ終了
    dragging	= false;
    ReleaseCapture();

    if(isLeftClick)
	{
        // ドラッグを行っていなくて、ダブルクリック間隔以上に押下状態が続いていたら
        if(timerState == 1)
	    {
            ShowSystemMenu();
            timerState = 0;
        }
    }
	else
	{
        KillSysMenuTimer();
    }

    return false;
}
// --------------------------------------------------------------------------
bool Core::OnLBUTTONUP(WPARAM wParam, LPARAM lParam){

    return OnBUTTONUP_Core(wParam, lParam);
}
// --------------------------------------------------------------------------
bool Core::OnRBUTTONUP(WPARAM wParam, LPARAM lParam){

    return OnBUTTONUP_Core(wParam, lParam);
}
// --------------------------------------------------------------------------
bool Core::OnTIMER(WPARAM wParam, LPARAM lParam){

    switch(wParam)
	{
        case WM_TIMER_SYSMENU:
	    {
            KillSysMenuTimer();

            if(dragging == FALSE)
	        {
                ShowSystemMenu();
            }
            timerState	= 1;
            break;
        }
    }

    return true;
}
// --------------------------------------------------------------------------
void Core::SetSysMenuTimer(){

    if(SetTimer(hwnd, WM_TIMER_SYSMENU, GetDoubleClickTime(), NULL) != 0)
    {
        isActiveSysmenuTimer = true;
    }
}
// --------------------------------------------------------------------------
void Core::KillSysMenuTimer(){

    if(isActiveSysmenuTimer)
    {
        isActiveSysmenuTimer = false;
        KillTimer(hwnd, WM_TIMER_SYSMENU);
    }
}
// --------------------------------------------------------------------------
void Core::ShowSystemMenu()
{
    int	iId	=
	    TrackPopupMenu(
		    GetSystemMenu(hwnd, false),
	        TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_TOPALIGN,
	        drawStartXpos,
	        drawStartYpos,
	        0,
	        hwnd,
	        NULL
	    );

    if(iId > 0)
    {
        PostMessage(hwnd, WM_SYSCOMMAND, (WPARAM)iId, 0);
    }
}
