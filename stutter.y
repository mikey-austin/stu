%{
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "gc.h"
#include "sv.h"
#include "env.h"

extern int yylex(void);
extern void yyerror(Sv **, char const *);

void yyerror (Sv **result, char const *s)
{
    warnx("%s", s);
}
%}

%union {
    long i;
    double f;
    char *str;
    Sv *sv;
}

%token <f>   FLOAT
%token <i>   INTEGER BOOLEAN
%token <str> STRING SYMBOL

%type <sv> list sexp forms atom elements

%start stutter
%parse-param { Sv **result }

%%

/* Just return the last sexp without evaling it. */
stutter:
    | forms                 { *result = $1; }
    ;

/* Eval every form in the order received except the last form. */
forms: forms sexp           { Sv_eval(MAIN_ENV, $1); $$ = $2; }
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
    ;

atom: INTEGER               { $$ = Sv_new_int($1); }
    | FLOAT                 { $$ = Sv_new_float($1); }
    | STRING                { $$ = Sv_new_str($1); }
    | SYMBOL                { $$ = Sv_new_sym($1); }
    | BOOLEAN               { $$ = Sv_new_bool((short) $1); }
    ;

%%
