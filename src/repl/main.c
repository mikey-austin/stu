#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <config.h>
#ifdef HAVE_EDITLINE_READLINE_H
#  include <editline/readline.h>
#endif

#include <stu.h>

extern char *optarg;
extern int optind, opterr, optopt;

int
main(int argc, char **argv)
{
    int option = 0, repl = 0, files = 0, debug = 0;
    StuVal *result = NULL;
    Stu *stu = Stu_new();

    while ((option = getopt(argc, argv, "rl:f:d")) != -1) {
        switch (option) {
        case 'f':
            files = 1;
            result = Stu_eval_file(stu, optarg);
            Stu_dump_sv(stu, result, stdout);
            printf("\n");
            break;

        case 'l':
            files = 1;
            Stu_eval_file(stu, optarg);
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
                result = Stu_eval_buf(stu, input);
                Stu_dump_sv(stu, result, stdout);
                printf("\n");
            }

            free(input);
        }
    }

    if (debug)
        Stu_dump_stats(stu, stderr);
    Stu_destroy(&stu);

    return 0;
}
