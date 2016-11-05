#include <readline.h>

#include "sv.h"
#include "parse.h"

int
main(int argc, char **argv)
{
    for (;;) {
        char *input = readline("stutter> ");
        add_history(input);

        Sv *result = Parse_buf(input);
        if (result)
            printf("parse ok\n");

        Sv_destroy(&result);
        if (result == NULL)
            printf("destroyed ok\n");
    }

    return 0;
}
