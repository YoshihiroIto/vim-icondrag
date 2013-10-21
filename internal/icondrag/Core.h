//=============================================================================
// FILE: Core.h
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
#ifndef CORE_H
#define CORE_H

#include <string>

class Core
{
public:
    static const char *PropertyName;

    Core(HWND hwnd);
    ~Core();

    void SetFilepath(const char *filepath);

private:
    bool        isDragging;
    bool        isActiveSysmenuTimer;
    bool        isLeftClick;
    int         timerState;
    int         drawStartXpos;
    int         drawStartYpos;

    std::string filepath;

    HWND        hwnd;
    WNDPROC     oldWndProc;
    HMODULE     selfModule;

    bool OnGETDATA(WPARAM wParam, LPARAM lParam);
    bool OnDESTROY(WPARAM wParam, LPARAM lParam);
    bool OnNCRBUTTONDOWN(WPARAM wParam, LPARAM lParam);
    bool OnNCLBUTTONDOWN(WPARAM wParam, LPARAM lParam);
    bool OnMOUSEMOVE(WPARAM wParam, LPARAM lParam);
    bool OnLBUTTONUP(WPARAM wParam, LPARAM lParam);
    bool OnRBUTTONUP(WPARAM wParam, LPARAM lParam);
    bool OnTIMER(WPARAM wParam, LPARAM lParam);

    bool OnNCBUTTONDOWN_Core(WPARAM wParam, LPARAM lParam);
    bool OnBUTTONUP_Core(WPARAM wParam, LPARAM lParam);

    void SetSysMenuTimer();
    void KillSysMenuTimer();
    void ShowSystemMenu();

    bool IsInDoubleClickRect();

    static LRESULT CALLBACK IconDragWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif

