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

#define EXPORT extern "C" __declspec(dllexport)

EXPORT char *IconDrag_Enable(const char *args)
{
    HWND hwnd = (HWND)args;

    if (GetPropA(hwnd, Core::PropertyName) != NULL)
    {
        return NULL;
    }

    Core *core = new Core(hwnd);
    SetPropA(hwnd, Core::PropertyName, core);

    return NULL;
}

EXPORT char *IconDrag_Disable(const char *args)
{
    HWND hwnd = (HWND)args;

    Core *core = (Core *)GetPropA(hwnd, Core::PropertyName);

    if (core == NULL)
    {
        return NULL;
    }

    //
    RemovePropA(hwnd, Core::PropertyName);

    delete core;

    return NULL;
}

EXPORT char *IconDrag_ClearCurrentFilepath(const char *args)
{
    HWND hwnd = (HWND)args;

    Core *core = (Core *)GetPropA(hwnd, Core::PropertyName);

    if (core == NULL)
    {
        return NULL;
    }

    core->SetFilepath("");

    return NULL;
}

EXPORT char *IconDrag_SetCurrentFilepath(const char *args)
{
    char *endptr;

    HWND hwnd = (HWND)strtol(args, &endptr, 0);
    const char *path = endptr + 1;

    Core *core = (Core *)GetPropA(hwnd, Core::PropertyName);

    if (core == NULL)
    {
        return NULL;
    }

    core->SetFilepath(path);

    return NULL;
}

