%option noyywrap
%option nodefault
%option nounput
%option noinput

%{
#include <stdio.h>
#include <string.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "svlist.h"
#include "stutter.tab.h"
%}

%%

"(" |
")" |
"'" |
"." { return yytext[0]; }

\"(\\.|[^"])*\"                     {
                                        char *dequoted = yytext + 1;
                                        dequoted[strlen(yytext) - 2] = '\0';
                                        yylval.str = dequoted;
                                        return STRING;
                                    }

-?[1-9][0-9]*\/[1-9][0-9]*          {
                                        char *str = yytext;
                                        Sv_rational r = {
                                            atol(strtok(str, "/")),
                                            atol(strtok(NULL, "/"))
                                        };
                                        yylval.rational = r;
                                        return RATIONAL;
                                    }

-?[0-9]+                            { yylval.i = atol(yytext); return INTEGER; }
-?[0-9]+\.[0-9]+                    { yylval.f = atof(yytext); return FLOAT; }
"#t"                                { yylval.i = 1; return BOOLEAN; }
"#f"                                { yylval.i = 0; return BOOLEAN; }

[a-zA-Z0-9._*+~#/=<>?!&\x80-\xf3-]+ { yylval.str = yytext; return SYMBOL; }

";".*

[ \t\n]

. { fprintf(stderr, "Unknown character %c\n", *yytext); }

%%
