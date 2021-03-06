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

%{
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "stu_private.h"

extern int yylex(void);
extern void yyerror(struct Stu *, Sv **, char const *);
extern int yyparse(struct Stu *, Sv **);

void yyerror (struct Stu *stu, Sv **list, char const *s)
{
    warnx("%s", s);
}
%}

%union {
    long i;
    double f;
    char *str;
    Sv_rational rational;
    Sv *sv;
}

%token    FIELD_ACCESSOR
%token <f>   FLOAT
%token <i>   INTEGER BOOLEAN
%token <str> STRING SYMBOL RE_SPEC RE_SPEC_I
%token <rational> RATIONAL

%type <sv> list vector sexp forms atom elements

%start stu
%parse-param { struct Stu *stu }
%parse-param { Sv **list }

%%

stu:
    | forms                 { *list = Sv_cons(stu, $1, *list); }
    ;

forms: forms sexp           { *list = Sv_cons(stu, $1, *list); $$ = $2; }
    | sexp                  { $$ = $1; }
    ;

vector: '[' ']'             { $$ = Sv_new_vector(stu, NIL); }
      | '[' elements ']'    { $$ = Sv_new_vector(stu, $2); }
      ;

list: '(' ')'               { $$ = NIL; }
    | '(' elements ')'      { $$ = $2; }
    | '(' sexp '.' sexp ')' { $$ = Sv_cons(stu, $2, $4); }
    ;

elements: sexp              { $$ = Sv_cons(stu, $1, NIL); }
    | sexp elements         { $$ = Sv_cons(stu, $1, $2); }
    ;

sexp: atom                  { $$ = $1; }
    | list                  { $$ = $1; }
    | vector                { $$ = $1; }
    | sexp FIELD_ACCESSOR sexp         { $$ = Sv_new_structure_access(stu, $1, $3); }
    | '\'' sexp             { $$ = Sv_cons(stu, Sv_new_sym(stu, "quote"), Sv_cons(stu, $2, NIL)); }
    | '`' sexp              { $$ = Sv_new_special(stu, SV_SPECIAL_BACKQUOTE, $2); }
    | ',' '@' sexp          { $$ = Sv_new_special(stu, SV_SPECIAL_COMMA_SPREAD, $3); }
    | ','  sexp             { $$ = Sv_new_special(stu, SV_SPECIAL_COMMA, $2); }
    ;

atom: INTEGER               { $$ = Sv_new_int(stu, $1); }
    | FLOAT                 { $$ = Sv_new_float(stu, $1); }
    | STRING                { $$ = Sv_new_str(stu, $1); }
    | SYMBOL                { $$ = Sv_new_sym(stu, $1); }
    | RE_SPEC               { $$ = Sv_new_regex(stu, $1, 0); }
    | RE_SPEC_I             { $$ = Sv_new_regex(stu, $1, 1); }
    | RATIONAL              { $$ = Sv_new_rational(stu, $1.n, $1.d); }
    | BOOLEAN               { $$ = Sv_new_bool(stu, (short) $1); }
    ;

%%
