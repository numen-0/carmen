
if exists("b:current_syntax")
  finish
endif

" keywords
syn keyword carmenKeyword
            \ if else loop while for break continue ret 
" Types
syn keyword carmenType
            \ void char byte short int long float double
            \ func proc struct enum
" normal
syn match carmenNormal /[a-zA-Z_][a-zA-Z_0-9]*/

" function
syn match carmenFunction /[a-zA-Z_][a-zA-Z_0-9]* *(/he=e-1

" comment tags
 syntax keyword carmenTodos contained TODO XXX FIXME NOTE HACK

" comments
syntax region carmenComment start="//" end="$"   contains=carmenTodos

" escape literals: \n, \r, \\, \', \0
syntax match carmenEscapes display contained "\\[0nr\']"

" char literals
syntax region carmenString start=/"/ skip=/\\\./ end=/"/ contains=carmenEscapes

" number literals
syn match carmenNumber /[0-9]\+/


" set highlights
highlight default link carmenComment           Comment
highlight default link carmenEscapes           SpecialChar
highlight default link carmenFunction          Function
highlight default link carmenKeyword           Keyword
highlight default link carmenNormal            Normal
highlight default link carmenNumber            Number
highlight default link carmenString            String
highlight default link carmenTodos             Todo
highlight default link carmenType              Type

let b:current_syntax = "carmen"

