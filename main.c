#ifdef __APPLE__
    #include <editline/readline.h>
#else
    #include <readline.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
    }

cleanup:
    Parse_cleanup();
    Gc_destroy_all();

    return 0;
}
