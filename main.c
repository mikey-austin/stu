#include <readline.h>
#include <stdio.h>
#include <stdlib.h>

#include "sv.h"
#include "parse.h"

int
main(int argc, char **argv)
{
    for (;;) {
        char *input = readline("stutter> ");
        if (input == NULL) {
            printf("\nBye!\n");
            break;
        }
        add_history(input);

        Sv *result = Parse_buf(input);
        if (result) {
            Sv_dump(result);
            printf("\n");
        }

        Sv_destroy(&result);
        free(input);
    }

    return 0;
}
