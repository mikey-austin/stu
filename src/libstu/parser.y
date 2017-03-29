%{
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "stu.h"
#include "gc.h"
#include "sv.h"
#include "env.h"
#include "svlist.h"

struct Stu;

extern int yylex(void);
extern void yyerror(struct Stu *, Svlist **, char const *);

void yyerror (struct Stu *stu, Svlist **list, char const *s)
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

%start stu
%parse-param { struct Stu *stu } { Svlist **list }

%%

stu:
    | forms                 { Svlist_push(*list, $1); }
    ;

forms: forms sexp           { Svlist_push(*list, $1); $$ = $2; }
    | sexp                  { $$ = $1; }
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
    | '\'' sexp             { $$ = Sv_cons(stu, Sv_new_sym(stu, "quote"), Sv_cons(stu, $2, NIL)); }
    | '`' sexp              { $$ = Sv_new_special(stu, SV_SPECIAL_BACKQUOTE, $2); }
    | ',' '@' sexp          { $$ = Sv_new_special(stu, SV_SPECIAL_COMMA_SPREAD, $3); }
    | ','  sexp             { $$ = Sv_new_special(stu, SV_SPECIAL_COMMA, $2); }
    ;

atom: INTEGER               { $$ = Sv_new_int(stu, $1); }
    | FLOAT                 { $$ = Sv_new_float(stu, $1); }
    | STRING                { $$ = Sv_new_str(stu, $1); }
    | SYMBOL                { $$ = Sv_new_sym(stu, $1); }
    | RATIONAL              { $$ = Sv_new_rational(stu, $1.n, $1.d); }
    | BOOLEAN               { $$ = Sv_new_bool(stu, (short) $1); }
    ;

%%
