%option noyywrap

%{
#include <string.h>
#include <stdio.h>
#include "stutter.tab.h"
%}

%%

"(" |
")" |
"'" { return yytext[0]; }

\"(\\.|[^"\n])*\" { yylval.str = strdup(yytext); return STRING; }

-?[0-9]+ { yylval.i = atoi(yytext); return INTEGER; }

[a-zA-Z0-9._+-/=<>!&]+ { yylval.sym = strdup(yytext); return SYMBOL; }

";".*

[ \t\n]

. { fprintf(stderr, "Unknown character %c\n", *yytext); }

%%
