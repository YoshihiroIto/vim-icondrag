//=============================================================================
// FILE: Core.cpp
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

extern "C"{
#include "OleDragDrop.h"
}

enum
{
    WM_TIMER_SYSMENU = 100,
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
    this->hwnd				= hwnd;
}
// --------------------------------------------------------------------------
void Core::Finalize()
{
    KillSysMenuTimer();
}
// --------------------------------------------------------------------------
void Core::SetFilepath(const char *filepath)
{
	this->filepath = filepath;
}
// --------------------------------------------------------------------------
bool Core::OnGETDATA(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
        case CF_HDROP:
	    {
			const DWORD	buffer_size	= sizeof(DROPFILES) + MAX_PATH + 1 + 1;
            //
            HDROP	drop_handle	= (HDROP)GlobalAlloc(GHND | GMEM_SHARE, buffer_size);
            {
                DROPFILES *	dropfiles	= (DROPFILES *)GlobalLock(drop_handle);

                ZeroMemory(dropfiles, buffer_size);
                dropfiles->pFiles	= sizeof(DROPFILES);		// �t�@�C�����̃��X�g�܂ł̃I�t�Z�b�g
                dropfiles->pt.x		= 0;
                dropfiles->pt.y		= 0;
                dropfiles->fNC		= false;
                dropfiles->fWide	= FALSE;
                CopyMemory(dropfiles + 1, filepath.c_str(), filepath.size());
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
    if(	(wParam == HTSYSMENU) &&
		(filepath.empty() == false))
	{
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
    isLeftClick	= true;				// ���N���b�N

    return OnNCBUTTONDOWN_Core(wParam, lParam);
}
// --------------------------------------------------------------------------
bool Core::OnNCRBUTTONDOWN(WPARAM wParam, LPARAM lParam)
{
    isLeftClick	= false;			// �E�N���b�N

    return OnNCBUTTONDOWN_Core(wParam, lParam);
}
// --------------------------------------------------------------------------
bool Core::OnMOUSEMOVE(WPARAM wParam, LPARAM lParam)
{
    if(dragging && (IsInDoubleClickRect() == false))
	{
        KillSysMenuTimer();

        // �h���b�O�J�n
        UINT	cf[]	= {CF_HDROP};
        int		iEffect	= (isLeftClick == true) ? DROPEFFECT_COPY : DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;

		OLE_IDropSource_Start(hwnd, (UINT)WM_GETDATA, cf, sizeof(cf) / sizeof(cf[0]), iEffect);

        // �h���b�O�I��
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
        (ABS(cursor_pos.x - drawStartXpos) < (GetSystemMetrics(SM_CXDOUBLECLK) / 2)) &&
        (ABS(cursor_pos.y - drawStartYpos) < (GetSystemMetrics(SM_CYDOUBLECLK) / 2));

    #undef ABS
}
// --------------------------------------------------------------------------
bool Core::OnBUTTONUP_Core(WPARAM wParam, LPARAM lParam){

    // �h���b�O�I��
    dragging	= false;
    ReleaseCapture();

    if(isLeftClick)
	{
        // �h���b�O���s���Ă��Ȃ��āA�_�u���N���b�N�Ԋu�ȏ�ɉ�����Ԃ������Ă�����
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
