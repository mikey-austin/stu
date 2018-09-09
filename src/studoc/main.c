/*
 * Copyright (c) 2018 Mikey Austin <mikey@jackiemclean.net>
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

extern char *optarg;
extern int optind, opterr, optopt;

int
main(int argc, char **argv)
{
    char *input = NULL;
    int option = 0;
    StuVal *forms = NULL;
    Stu *stu = Stu_new();

    while ((option = getopt(argc, argv, "rl:f:dL:")) != -1) {
        switch (option) {
        case 'L':
            Stu_add_include_path(stu, optarg);
            break;
        }
    }

    Stu_destroy(&stu);

    return 0;
}
