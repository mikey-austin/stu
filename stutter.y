%{
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "svlist.h"

extern int yylex(void);
extern void yyerror(Svlist **, char const *);

void yyerror (Svlist **list, char const *s)
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

%token <f>   FLOAT
%token <i>   INTEGER BOOLEAN
%token <str> STRING SYMBOL
%token <rational> RATIONAL

%type <sv> list sexp forms atom elements

%start stutter
%parse-param { Svlist **list }

%%

stutter:
    | forms                 { Svlist_push(*list, $1); }
    ;

forms: forms sexp           { Svlist_push(*list, $1); $$ = $2; }
    | sexp                  { $$ = $1; }
    ;

list: '(' ')'               { $$ = NIL; }
    | '(' elements ')'      { $$ = $2; }
    | '(' sexp '.' sexp ')' { $$ = Sv_cons($2, $4); }
    ;

elements: sexp              { $$ = Sv_cons($1, NIL); }
    | sexp elements         { $$ = Sv_cons($1, $2); }
    ;

sexp: atom                  { $$ = $1; }
    | list                  { $$ = $1; }
    | '\'' sexp             { $$ = Sv_cons(Sv_new_sym("quote"), Sv_cons($2, NIL)); }
    | '`' sexp              { $$ = Sv_new_special(SV_SPECIAL_BACKQUOTE, $2); }
    | ',' '@' sexp          { $$ = Sv_new_special(SV_SPECIAL_COMMA_SPREAD, $3); }
    | ','  sexp             { $$ = Sv_new_special(SV_SPECIAL_COMMA, $2); }
    ;

atom: INTEGER               { $$ = Sv_new_int($1); }
    | FLOAT                 { $$ = Sv_new_float($1); }
    | STRING                { $$ = Sv_new_str($1); }
    | SYMBOL                { $$ = Sv_new_sym($1); }
    | RATIONAL              { $$ = Sv_new_rational($1.n, $1.d); }
    | BOOLEAN               { $$ = Sv_new_bool((short) $1); }
    ;

%%
