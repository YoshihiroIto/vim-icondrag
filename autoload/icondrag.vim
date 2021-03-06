"=============================================================================
" FILE: icondrag.vim
" AUTHOR: Yoshihiro Ito <yo.i.jewelry.bab@gmail.com@gmail.com>
" License: MIT license  {{{
"     Permission is hereby granted, free of charge, to any person obtaining
"     a copy of this software and associated documentation files (the
"     "Software"), to deal in the Software without restriction, including
"     without limitation the rights to use, copy, modify, merge, publish,
"     distribute, sublicense, and/or sell copies of the Software, and to
"     permit persons to whom the Software is furnished to do so, subject to
"     the following conditions:
"
"     The above copyright notice and this permission notice shall be included
"     in all copies or substantial portions of the Software.
"
"     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
"     OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
"     MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
"     IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
"     CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
"     TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
"     SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
" }}}
"=============================================================================
if has('win64')
    let s:icondrag_dll = expand('<sfile>:p:h') . '\icondrag64.dll'
else
    let s:icondrag_dll = expand('<sfile>:p:h') . '\icondrag32.dll'
endif

if !exists('s:icondrag_state')
    let s:icondrag_state = 0
endif

let s:isWindows = has('win32') || has('win64')

function! icondrag#enable()
    if !s:isWindows
        return
    endif

    call libcallnr(s:icondrag_dll, 'IconDrag_Enable', v:windowid)
    let s:icondrag_state = 1

    call icondrag#set_current_filepath()
endfunction

function! icondrag#disable()
    if !s:isWindows
        return
    endif

    call libcallnr(s:icondrag_dll, 'IconDrag_Disable', v:windowid)
    let s:icondrag_state = 0
endfunction

function! icondrag#set_current_filepath()
    if !s:isWindows
        return
    endif

    if s:icondrag_state == 1
        let filepath = expand('%:p')

        if filepath == ''
            call libcallnr(s:icondrag_dll, 'IconDrag_ClearCurrentFilepath', v:windowid)
        else
            call libcallnr(s:icondrag_dll, 'IconDrag_SetCurrentFilepath', printf('0x%X,%s', v:windowid, filepath))
        endif
    endif
endfunction

