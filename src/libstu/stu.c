#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "gc.h"
#include "sv.h"
#include "svlist.h"
#include "lexer.h"
#include "parser.h"
#include "stu.h"

extern Svlist
*Stu_parse_buf(const char *buf)
{
    Svlist *list = Svlist_new();

    if (buf) {
        YY_BUFFER_STATE bp = yy_scan_string(buf);
        yy_switch_to_buffer(bp);
        switch (yyparse(&list)) {
        case 2:
            errx(1, "Parser memory allocation error");
            break;
        }
        yy_delete_buffer(bp);
    }

    return list;
}

extern Svlist
*Stu_parse_file(const char *file)
{
    Svlist *list = Svlist_new();

    if (!file || !strcmp(file, "-")) {
        yyin = stdin;
    } else {
        if ((yyin = fopen(file, "r")) == NULL)
            err(1, "Parse_file");
    }

    switch (yyparse(&list)) {
    case 2:
        errx(1, "Parser memory allocation error");
        break;
    }

    if (yyin && yyin != stdin)
        fclose(yyin);
    yylex_destroy();

    return list;
}
