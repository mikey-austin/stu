%{
#include <stdio.h>
#include <stdlib.h>

extern int yylex(void);
extern void yyerror(char const *s);

void yyerror (char const *s)
{
    fprintf (stderr, "%s\n", s);
}
%}

%union {
    int i;
    char *str;
    char *sym;
}

/* declare tokens */
%token  <i>             INTEGER
%token  <str>           STRING
%token  <sym>           SYMBOL

%start stutter

%%

stutter:                { printf("nothing\n"); }
    | list              { printf("stutter list\n"); }
    ;

sexp: atom           { printf("sexp\n"); }
    | list
    ;

list: '(' elements ')'  { printf("list\n"); }
    | '(' ')'
    ;

elements: sexp
    | sexp elements
    ;

atom: INTEGER            { printf("integer: %d\n", $1); }
    | STRING             { printf("string: %s\n", $1); }
    | SYMBOL             { printf("symbol: %s\n", $1); }
    ;

%%

int main(void) {
    yyparse();
    return 0;
}
