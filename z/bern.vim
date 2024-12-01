" adapted from yami


set background=dark
highlight clear
if exists("syntax_on")
	syntax reset
endif
let g:colors_name = "bern"

" Color palette
" #d4d4d5
let s:light  = "#ffffff"
let s:dark   = "#000000"
let s:accent = "#878787"
let s:gray0  = "#666666"
let s:gray1  = "#323232"
let s:gray2  = "#23242a"
let s:gray3  = "#1b1b1b"
let s:red    = "#f92672"
let s:orange = "#fd971f"
let s:yellow = "#e6db74"
let s:green  = "#a6e22e"
let s:blue   = "#66d9ef"
let s:purple = "#ae81ff"


" Highlight helper function
function! s:HL(item, fgColor, bgColor, style)
	let command  = 'hi ' . a:item
	let command .= ' ' . 'gui' . 'fg=' . a:fgColor
	let command .= ' ' . 'gui' . 'bg=' . a:bgColor
	let command .= ' ' . 'gui' . '=' . a:style
	execute command
endfunction

call s:HL('Folded'		  , s:gray3, '#000000' , 'NONE' )
call s:HL('MatchParen'	  , s:dark, s:gray0 , 'NONE' )

call s:HL('Normal'     , s:light , s:dark  , 'NONE'      )
call s:HL('Cursor'     , s:dark  , s:accent, 'NONE'      )
call s:HL('Visual'    , 'NONE'   , s:gray0 , 'NONE' )

" quickfix window (for some reason)
call s:HL('Search'    , s:light , s:dark , 'bold' )

" Popup Menu
call s:HL('Pmenu'      , s:light , s:accent , 'NONE' )
call s:HL('PmenuSbar'  , s:dark  , s:dark , 'NONE' )
call s:HL('PmenuSel'   , s:dark  , s:light , 'NONE' )
call s:HL('PmenuThumb' , s:dark  , s:light , 'NONE' )

" Special
call s:HL('cDefine'      , s:accent  , 'NONE' , 'bold' )

" Primitives
call s:HL('String'      , s:light, 'NONE' , 'NONE' )
call s:HL('Character'   , s:light , 'NONE' , 'NONE' )
call s:HL('SpecialChar' , s:light , 'NONE' , 'NONE' )
call s:HL('Number'      , s:light , 'NONE' , 'NONE' )
call s:HL('Boolean'     , s:light , 'NONE' , 'NONE' )
call s:HL('Float'       , s:light , 'NONE' , 'NONE' )
call s:HL('Constant'    , s:light , 'NONE' , 'NONE' )

" Specials
call s:HL('Title'          , s:gray0  , 'NONE' , 'NONE' )
call s:HL('Todo'           , s:dark , s:light , 'bold' )
call s:HL('Comment'        , s:dark , s:light , 'NONE' )
" call s:HL('SpecialComment' , s:green , 'NONE' , 'NONE' )


" Lines                  , Columns
call s:HL('LineNr'       , s:gray2 , 'NONE'  , 'NONE' )
call s:HL('CursorLine'   , 'NONE'  , 'NONE' , 'NONE' )
call s:HL('CursorLineNr' , s:accent , 'NONE', 'NONE' )
call s:HL('SignColumn'   , s:gray3 , s:dark  , 'NONE' )
call s:HL('ColorColumn'  , s:light , s:gray3 , 'NONE' )
call s:HL('CursorColumn' , s:light , s:gray3 , 'NONE' )

" Visual
call s:HL('VisualNOS' , s:gray3  , s:light , 'NONE' )
call s:HL('IncSearch' , s:red , s:dark , 'NONE' )

" Messages
call s:HL('ErrorMsg'   , s:red    , s:dark , 'NONE' )
call s:HL('WarningMsg' , s:yellow , s:dark , 'NONE' )
call s:HL('ModeMsg'    , s:green  , s:dark , 'NONE' )
call s:HL('MoreMsg'    , s:green  , s:dark , 'NONE' )
call s:HL('Error'      , s:red    , s:dark , 'standout' )
call s:HL('Warning'    , s:orange , s:dark , 'standout' )
call s:HL('qfNote'     , s:accent , s:dark , 'standout' )

" Preprocessor Directives
call s:HL('Include'		  , s:light	, 'NONE', 'NONE' )
call s:HL('Define'		  , s:light	, 'NONE', 'NONE' )
call s:HL('Macro'		  , s:light	, 'NONE', 'NONE' )
call s:HL('PreCondit'	  , s:light	, 'NONE', 'NONE' )
call s:HL('PreProc'		  , s:light	, 'NONE', 'NONE' )

" Bindings
call s:HL('Identifier'	  , s:light	, 'NONE', 'NONE' )
call s:HL('Function'	  , s:light	, 'NONE', 'NONE' )
call s:HL('Keyword'		  , s:light	, 'NONE', 'NONE' )
call s:HL('Operator'	  , s:light	, 'NONE', 'NONE' )

" Types
call s:HL('Type'		  , s:light	, 'NONE', 'NONE' )
call s:HL('Typedef'	  	  , s:light	, 'NONE', 'NONE' )
call s:HL('StorageClass'  , s:light	, 'NONE', 'NONE' )
call s:HL('Structure'	  , s:light	, 'NONE', 'NONE' )

" Flow Control
call s:HL('Statement'	  , s:light	, 'NONE', 'NONE' )
call s:HL('Conditional'	  , s:light	, 'NONE', 'NONE' )
call s:HL('Repeat'		  , s:light	, 'NONE', 'NONE' )
call s:HL('Label'		  , s:light	, 'NONE', 'NONE' )
call s:HL('Exception'	  , s:light	, 'NONE', 'NONE' )

call s:HL('cBold'		  , s:light	, 'NONE', 'underline' )

" Misc
call s:HL('Underlined' , s:light , 'NONE'  , 'underline' )
call s:HL('SpecialKey' , s:light , 'NONE'  , 'NONE'      )
call s:HL('NonText'    , s:dark , 'NONE'  , 'NONE'      )
call s:HL('Directory'  , s:light , 'NONE'  , 'NONE'      )

" Fold
call s:HL('FoldColumn'	  , s:gray3, 'NONE' , 'italic' )


" Split
call s:HL('VertSplit', s:dark, s:dark , 'bold' )













" Others
call s:HL('Debug'        , s:light , 'NONE'  , 'NONE' )
call s:HL('Delimiter'    , s:light , 'NONE'  , 'NONE' )
call s:HL('Question'     , s:light , 'NONE'  , 'NONE' )
call s:HL('Special'      , s:light , 'NONE'  , 'NONE' )
call s:HL('StatusLine'   , s:gray1 , s:dark , 'NONE' )
call s:HL('StatusLineNC' , s:gray3, s:dark , 'NONE' )
call s:HL('Tag'          , s:light , 'NONE'  , 'NONE' )
call s:HL('WildMenu'     , s:dark  , s:light , 'NONE' )
call s:HL('TabLine'      , s:light , s:gray2 , 'NONE' )

" Diff
call s:HL('DiffAdd'    , s:green  , 'NONE' , 'NONE' )
call s:HL('DiffChange' , s:yellow , 'NONE' , 'NONE' )
call s:HL('DiffDelete' , s:red    , 'NONE' , 'NONE' )
call s:HL('DiffText'   , s:dark   , 'NONE' , 'NONE' )

" GitGutter
call s:HL('GitGutterAdd'          , s:green  , 'NONE' , 'NONE' )
call s:HL('GitGutterChange'       , s:yellow , 'NONE' , 'NONE' )
call s:HL('GitGutterDelete'       , s:red    , 'NONE' , 'NONE' )
call s:HL('GitGutterChangeDelete' , s:dark   , 'NONE' , 'NONE' )

" Vimscript
call s:HL('vimFunc'          , s:light , 'NONE' , 'NONE' )
call s:HL('vimUserFunc'      , s:light , 'NONE' , 'NONE' )
call s:HL('vimLineComment'   , s:gray0 , 'NONE' , 'NONE' )
call s:HL('vimCommentString' , s:gray0 , 'NONE' , 'NONE' )

" NERDTree
call s:HL('NERDTreeCWD'            , s:gray1 , 'NONE' , 'NONE' )
call s:HL('NERDTreeFile'           , s:light , 'NONE' , 'NONE' )
call s:HL('NERDTreeNodeDelimiters' , s:light , 'NONE' , 'NONE' )


" FZF
call s:HL('fzf1' , s:light , s:gray2 , 'NONE' )
call s:HL('fzf2' , s:light , s:gray2 , 'NONE' )
call s:HL('fzf3' , s:light , s:gray2 , 'NONE' )

" Spelling
call s:HL('SpellBad'   , s:red , s:dark , 'NONE' )
call s:HL('SpellCap'   , s:red , s:dark , 'NONE' )
call s:HL('SpellLocal' , s:red , s:dark , 'NONE' )
call s:HL('SpellRare'  , s:red , s:dark , 'NONE' )

call s:HL('Conceal'           , s:accent ,  s:gray3 , 'NONE' )
call s:HL('cRegion'           , s:accent ,  s:gray3 , 'NONE' )
call s:HL('cRegion2'           , s:light ,  s:gray3 , 'bold' )
