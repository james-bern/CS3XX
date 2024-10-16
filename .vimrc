autocmd FileWritePost *.java asdf

" Use spaces instead of tabs
set expandtab
" Be smart when using tabkey
set smarttab
" 1 tab == 4 spaces
set shiftwidth=4
set tabstop=4
" Enable filetype plugins
filetype plugin on
filetype indent on
set ai "Auto indent
" set si "Smart indent
set wrap "Wrap lines
" Set to auto read when a file is changed from the outside
set autoread
" Configure backspace so it acts as it should act
set backspace=eol,start,indent
" Show matching brackets when text indicator is over them
set showmatch 
" How many tenths of a second to blink when matching brackets
set mat=2
" No annoying sound on errors
set noerrorbells
set novisualbell
set t_vb=
set tm=500
" Enable syntax highlighting
syntax enable 
" Set utf8 as standard encoding and en_US as the standard language
set encoding=utf8
" Use Unix as the standard file type
set ffs=unix,dos,mac
" Turn backup off, since most stuff is in SVN, git et.c anyway...
set nobackup
set nowb
set noswapfile
" Return to last edit position when opening files (You want this!)
autocmd BufReadPost *
     \ if line("'\"") > 0 && line("'\"") <= line("$") |
     \   exe "normal! g`\"" |
     \ endif
" Remember info about open buffers on close
set viminfo^=%
filetype plugin indent on
syntax on
set foldmethod=indent
set number
imap <Escape> <Escape>:w<Enter>

"" leader
nmap <space> <leader>
vmap <space> <leader>
set foldminlines=0
set foldignore=#
" Return to last edit position when opening files (You want this!)
autocmd BufReadPost *
            \ if line("'\"") > 0 && line("'\"") <= line("$") |
            \   exe "normal! g`\"" |
            \ endif
" TODO: kill end of line
function! MyFoldText()
    let nucolwidth = &fdc + &number * &numberwidth
    let windowwidth = winwidth(0) - nucolwidth
    let first_line = getline(v:foldstart)
    let last_line = getline(v:foldend)
    let spaces = match(first_line,'\S')
    let pluses = v:foldend - v:foldstart + 1
    return repeat(" ", spaces) . repeat("·", pluses) . repeat (" ", windowwidth - spaces - pluses)
endfunction
set foldtext=MyFoldText()
" (un)fold
map <leader><space> za
map <leader><S-space> zA


" txt stuff
autocmd BufWinEnter *.txt silent!
            \ | set nopaste
            \ | set wrap
            \ | set linebreak
            \ | set tabstop=1
            \ | set shiftwidth=1
            \ | set softtabstop=1

autocmd BufWinEnter *.tex,*.c,*.h,*.cpp,*.cxx,*.hpp,*.md,*.java,*.ino silent!
            \ | set nopaste
            \ | set wrap
            \ | set tabstop=4
            \ | set shiftwidth=4
            \ | set softtabstop=4
            \ | set formatoptions-=cro


" FORNOW: java compilation


"" auto-indent
set cindent
set cinkeys=0{,0},0),:,!^F,o,O,e
"set cinoptions+=#1s
filetype plugin indent on
" https://stackoverflow.com/questions/2506776/is-it-possible-to-format-c-code-with-vim
" set equalprg=astyle\ -o\ -O\ -p\ -xg\ -H\ -xe\ -W3\ -j\ -J\ -w\ -xW\ -xw\ -w\ -A2
" https://stackoverflow.com/questions/15992163/how-to-tell-vim-to-auto-indent-before-saving
function! AlexanderShukaevPreserve(command)
    " Save the last search.
    let search = @/
    " Save the current cursor position.
    let cursor_position = getpos('.')
    " Save the current window position.
    normal! H
    let window_position = getpos('.')
    call setpos('.', cursor_position)
    " Execute the command.
    execute a:command
    " Restore the last search.
    let @/ = search
    " Restore the previous window position.
    call setpos('.', window_position)
    normal! zt
    " Restore the previous cursor position.
    call setpos('.', cursor_position)
endfunction
function! Indent()
    call AlexanderShukaevPreserve('normal gg=G')
endfunction
" autocmd BufWritePre *.cpp,*.ino call Indent()
map <leader><tab> :set foldignore= <CR>:call Indent()<CR>
" imap <space><tab> <Esc><leader><tab>

map <leader><tab> :set foldignore= <CR>:call Indent()<CR>

autocmd VimEnter *.* silent!
            \ | set foldignore=" "

"commentary
vmap <leader>c gc
nmap <leader>c gcc

" " substitution
" replace current word
nmap <leader>s :%s/\<<C-r><C-w>\>//g \| w<left><left><left><left><left><left>
nmap <leader><c-s> :argdo %s/\<<C-r><C-w>\>//g \| w<left><left><left><left><left><left>
" replace current word _inside previous visual selection_
nmap <leader>S <Esc>:'<,'>s/\<<C-r><C-w>\>//g<left><left>
" replace visual selection
vmap <leader>s "hy:%s/\V<C-r>h//g<left><left>
" replace inside visual selection
vmap <leader>S <Esc>:'<,'>s///g<left><left><left>

" " vimgrep
" replace current word
nmap <leader>f :vimgrep '' *.*<left><left><left><left><left>
vmap <leader>f "hy:vimgrep '\V<C-r>h' *.*<CR>

nmap <leader>e :e<Space>
vmap <leader>e <Esc><leaver>e
nmap <leader>v :vs<CR><C-w><C-l>:e<Space>
vmap <leader>v <Esc><leader>v

color quiet
" set up windows for terminal on top and quickfix on right 
autocmd VimEnter *.java,*.c,*.cpp,*.h,*.hpp  silent!
            \ | vertical copen 
            \ | set nowrap
            \ | wincmd K 
            \ | wincmd j 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
            \ | wincmd + 
autocmd VimEnter *.java silent!
            \ | wincmd n
            \ | wincmd j 
autocmd VimEnter *.cpp  silent!
            \ | set background=dark

" map Control + Enter to compile and run
"https://stackoverflow.com/questions/6411979/compiling-java-code-in-vim-more-efficiently
map <C-Enter> :wa<CR>:call CompileAndRun()<CR>
imap <C-Enter> <Esc><C-Enter>
func! CompileAndRun()
exec "w"
if &filetype == 'java'
silent exec "! rm *.class"
silent exec "set makeprg=javac"
silent exec "make! % Cow.java"
exec "let className = expand('%:r')"
wincmd k
exec "terminal! ++curwin java -Djava.compiler=NONE -ea " . className
wincmd j
else
silent exec "set makeprg=./build_and_run.bat"
silent exec "make! main.cpp"
endif
endfunc

autocmd FileType c,cpp,cs,java,kotlin setlocal commentstring=//\ %s

set nosmartindent
set cindent
filetype plugin indent on
set cinkeys-=0#
set indentkeys-=0#
autocmd FileType * set cindent "some file types override it
