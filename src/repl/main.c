#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <editline/readline.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "symtab.h"
#include "stu.h"
#include "builtins.h"

extern char *optarg;
extern int optind, opterr, optopt;

int
main(int argc, char **argv)
{
    int option = 0, repl = 0, files = 0, debug = 0;
    Svlist *forms = NULL;
    Stu *stu = Stu_new();

    while ((option = getopt(argc, argv, "rl:f:d")) != -1) {
        switch (option) {
        case 'f':
            files = 1;
            PUSH_SCOPE(stu);
            forms = Stu_parse_file(stu, optarg);
            Sv_dump(stu, Svlist_eval(stu, stu->main_env, forms));
            Svlist_destroy(&forms);
            POP_SCOPE(stu);
            printf("\n");
            break;

        case 'l':
            files = 1;
            PUSH_SCOPE(stu);
            forms = Stu_parse_file(stu, optarg);
            Svlist_eval(stu, stu->main_env, forms);
            Svlist_destroy(&forms);
            POP_SCOPE(stu);
            break;

        case 'r':
            repl = 1;
            break;

        case 'd':
            debug = 1;
            break;
        }
    }

    if (!files || repl) {
        for (;;) {
            char *input = readline("stu> ");
            if (input == NULL) {
                printf("\nBye!\n");
                break;
            }

            if (*input != '\0') {
                add_history(input);
                PUSH_SCOPE(stu);
                forms = Stu_parse_buf(stu, input);
                Sv_dump(stu, Svlist_eval(stu, stu->main_env, forms));
                Svlist_destroy(&forms);
                POP_SCOPE(stu);
                printf("\n");
            }

            free(input);
        }
    }

    if (debug)
        Gc_dump_stats(stu);
    Stu_destroy(&stu);

    return 0;
}
