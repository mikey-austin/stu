%option noyywrap
%option nodefault
%option nounput
%option noinput

%{
#include <stdio.h>
#include "sv.h"
#include "stutter.tab.h"
%}

%%

"(" |
")" |
"'" |
"." { return yytext[0]; }

\"(\\.|[^"])*\" { yylval.str = yytext; return STRING; }

-?[0-9]+ { yylval.i = atol(yytext); return INTEGER; }
"#t"     { yylval.i = 1; return BOOLEAN; }
"#f"     { yylval.i = 0; return BOOLEAN; }

[a-zA-Z0-9._*+-/=<>?!&]+ { yylval.str = yytext; return SYMBOL; }

";".*

[ \t\n]

. { fprintf(stderr, "Unknown character %c\n", *yytext); }

%%
