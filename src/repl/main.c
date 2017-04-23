/*
 * Copyright (c) 2016, 2017 Mikey Austin <mikey@jackiemclean.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <config.h>
#include <libstu/stu.h>

#include "prompt.h"

extern char *optarg;
extern int optind, opterr, optopt;

int
main(int argc, char **argv)
{
    char *input = NULL;
    int option = 0, repl = 0, files = 0, debug = 0;
    StuVal *result = NULL;
    Stu *stu = Stu_new();

    while ((option = getopt(argc, argv, "rl:f:d")) != -1) {
        switch (option) {
        case 'f':
            files = 1;
            result = Stu_eval_file(stu, optarg);
            Stu_dump_val(stu, result, stdout);
            Stu_release_val(stu, result);
            printf("\n");
            break;

        case 'l':
            files = 1;
            result = Stu_eval_file(stu, optarg);
            Stu_release_val(stu, result);
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
        Prompt_init(argv[0]);

        for (;;) {
            int nread = Prompt_readline(&input);
            if (input == NULL) {
                printf("\nBye!\n");
                break;
            }

            if (*input != '\0' && *input != '\n') {
                result = Stu_eval_buf(stu, input);
                Stu_dump_val(stu, result, stdout);
                Stu_release_val(stu, result);
                Prompt_save(input);
                printf("\n");
            }

            free(input);
            input = NULL;
        }

        Prompt_finish();
    }

    if (debug)
        Stu_dump_stats(stu, stderr);
    Stu_destroy(&stu);

    return 0;
}
