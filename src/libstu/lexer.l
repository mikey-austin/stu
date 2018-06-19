/*
 * Copyright (c) 2016, 2017 Mikey Austin <mikey@jackiemclean.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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
#include "stu_private.h"
#include "parser.h"
%}

%%

"[" |
"]" |
"(" |
")" |
"'" |
"`" |
"," |
"@" |
"." { return yytext[0]; }

\"(\\.|[^"])*\"                      {
                                         char *dequoted = yytext + 1;
                                         dequoted[strlen(yytext) - 2] = '\0';
                                         yylval.str = dequoted;
                                         return STRING;
                                     }

-?[1-9][0-9]*\/[1-9][0-9]*           {
                                         char *str = yytext;
                                         Sv_rational r = {
                                             atol(strtok(str, "/")),
                                             atol(strtok(NULL, "/"))
                                         };
                                         yylval.rational = r;
                                         return RATIONAL;
                                     }

-?[0-9]+                             { yylval.i = atol(yytext); return INTEGER; }
-?[0-9]+\.[0-9]+                     { yylval.f = atof(yytext); return FLOAT; }
"#t"                                 { yylval.i = 1; return BOOLEAN; }
"#f"                                 { yylval.i = 0; return BOOLEAN; }

[a-zA-Z0-9._*+~#/$=<>?!&\x80-\xf3-]+ { yylval.str = yytext; return SYMBOL; }

";".*

[ \t\n]

.

%%
