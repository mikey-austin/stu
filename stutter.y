%{
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include "sv.h"

extern int yylex(void);
extern void yyerror(Sv **result, char const *s);

void yyerror (Sv **result, char const *s)
{
    warnx("%s", s);
}
%}

%union {
    long i;
    char *str;
    Sv *sv;
}

%token <i>   INTEGER
%token <str> STRING SYMBOL SPECIAL

%type <sv> list sexp atom elements special

%start stutter
%parse-param {
    Sv **result
}

%%

stutter:
    | sexp                  { *result = $1; }
    ;

list: '(' ')'               { $$ = NULL; }
    | '(' elements ')'      { $$ = $2; }
    | '(' sexp '.' sexp ')' { $$ = Sv_cons($2, $4); }
    | '(' special sexp ')'  { $$ = Sv_special($2, $3); }
    | '\'' list             { $$ = Sv_special(Sv_new_sym("quote"), $2); }
    ;

elements: sexp              { $$ = Sv_cons($1, NULL); }
    | sexp elements         { $$ = Sv_cons($1, $2); }
    ;

sexp: atom                  { $$ = $1; }
    | list                  { $$ = $1; }
    | '\'' atom             { $$ = Sv_special(Sv_new_sym("quote"), $2); }
    ;

special: SPECIAL            { $$ = Sv_new_sym($1); }
    ;

atom: INTEGER               { $$ = Sv_new_int($1); }
    | STRING                { $$ = Sv_new_str($1); }
    | SYMBOL                { $$ = Sv_new_sym($1); }
    ;

%%
