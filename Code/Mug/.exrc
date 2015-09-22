if &cp | set nocp | endif
nmap e :cw
let s:cpo_save=&cpo
set cpo&vim
nmap gx <Plug>NetrwBrowseX
nmap mp :make program
nmap m :w :make
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
let &cpo=s:cpo_save
unlet s:cpo_save
set backspace=2
set fileencodings=ucs-bom,utf-8,default,latin1
set helplang=nl
set modelines=0
set window=0
" vim: set ft=vim :
