#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <editline/readline.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "symtab.h"
#include "parse.h"
#include "builtins.h"

extern char *optarg;
extern int optind, opterr, optopt;

int
main(int argc, char **argv)
{
    int option = 0, repl = 0, files = 0;
    Sv *result = NULL;

    Symtab_init();
    Builtin_init();
    Sv_init();

    while((option = getopt(argc, argv, "rl:f:")) != -1) {
        switch(option) {
        case 'f':
            files = 1;
            result = Parse_file(optarg);
            Sv_dump(Sv_eval(MAIN_ENV, result));
            printf("\n");
            break;

        case 'l':
            files = 1;
            Sv_eval(MAIN_ENV, Parse_file(optarg));
            break;

        case 'r':
            repl = 1;
            break;
        }
    }

    if (!files || repl) {
        for (;;) {
            char *input = readline("stutter> ");
            if (input == NULL) {
                printf("\nBye!\n");
                break;
            }

            if (*input != '\0') {
                add_history(input);
                result = Parse_buf(input);
                Sv_dump(Sv_eval(MAIN_ENV, result));
                printf("\n");
            }

            free(input);
            Gc_collect();
        }
    }

    Gc_sweep(0);
    Gc_dump_stats();
    Symtab_destroy();

    return 0;
}
