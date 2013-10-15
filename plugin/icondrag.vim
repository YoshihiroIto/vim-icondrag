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

if (exists('g:loaded_icondrag') && g:loaded_icondrag) || !has('win32') || !has('gui_running')
" if (exists('g:loaded_icondrag') && g:loaded_icondrag) || !has('gui_running')
    finish
endif
let g:loaded_icondrag = 1

command! -nargs=0 -bar IconDragEnable  call icondrag#enable()
command! -nargs=0 -bar IconDragDisable call icondrag#disable()

augroup icondrag_plugin
    autocmd!
    if g:icondrag_auto_start
        autocmd GuiEnter * call icondrag#enable()
    endif
augroup END

