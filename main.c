#include <readline.h>
#include <stdio.h>
#include <stdlib.h>

#include "sv.h"
#include "env.h"
#include "parse.h"
#include "builtins.h"

int
main(int argc, char **argv)
{
    Env *env = Env_new();
    Builtin_install(env);

    for (;;) {
        char *input = readline("stutter> ");
        if (input == NULL) {
            printf("\nBye!\n");
            break;
        }
        add_history(input);

        Sv *result = Parse_buf(input);
        if (result) {
            Sv_dump(Sv_eval(env, result));
            printf("\n");
        }

        Sv_destroy(&result);
        free(input);
    }

    return 0;
}
