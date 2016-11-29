#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "gc.h"
#include "env.h"
#include "sv.h"
#include "lex.yy.h"
#include "stutter.tab.h"
#include "parse.h"

extern Sv
*Parse_buf(Env *env, const char *buf)
{
    Sv *result = NULL;

    if (buf) {
        YY_BUFFER_STATE bp = yy_scan_string(buf);
        yy_switch_to_buffer(bp);
        switch (yyparse(env, &result)) {
        case 2:
            errx(1, "Parser memory allocation error");
            break;
        }
        yy_delete_buffer(bp);
    }

    return result;
}

extern Sv
*Parse_file(Env *env, const char *file)
{
    Sv *result = NULL;

    if (!file || !strcmp(file, "-")) {
        yyin = stdin;
    } else {
        if ((yyin = fopen(file, "r")) == NULL)
            err(1, "Parse_file");
    }

    switch (yyparse(env, &result)) {
    case 2:
        errx(1, "Parser memory allocation error");
        break;
    }

    if (yyin && yyin != stdin)
        fclose(yyin);
    yylex_destroy();

    return result;
}
