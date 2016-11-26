#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <editline/readline.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "parse.h"
#include "builtins.h"

extern char *optarg;
extern int optind, opterr, optopt;

int
main(int argc, char **argv)
{
    int option;
    Env *env = Env_new();
    Sv *result = NULL;

    Gc_init((Gc *) env);
    Builtin_install(env);

    while((option = getopt(argc, argv, "f:")) != -1) {
        switch(option) {
        case 'f':
            result = Parse_file(env, optarg);
            Sv_dump(Sv_eval(env, result));
            printf("\n");
            goto cleanup;
        }
    }

    for (;;) {
        char *input = readline("stutter> ");
        if (input == NULL) {
            printf("\nBye!\n");
            break;
        }

        if (*input != '\0') {
            add_history(input);
            result = Parse_buf(env, input);
            Sv_dump(Sv_eval(env, result));
            printf("\n");
        }

        free(input);
        Gc_collect();
    }

cleanup:
    rl_free_line_state();
    Parse_cleanup();
    Gc_dump_stats();
    Gc_sweep(0);

    return 0;
}
