#include <stdlib.h>
#include <err.h>
#include "sv.h"
#include "lex.yy.h"
#include "stutter.tab.h"
#include "parse.h"

extern Sv
*Parse_buf(const char *buf)
{
    Sv *result = NULL;

    if (buf) {
        YY_BUFFER_STATE bp = yy_scan_string(buf);

        yy_switch_to_buffer(bp);
        switch (yyparse(&result)) {
        case 2:
            errx(1, "Parser memory allocation error");
            break;
        }
    }

    return result;
}
