/*
 * Copyright (c) 2017 Mikey Austin <mikey@jackiemclean.net>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>

#include <libstu/stu.h>
#include "prompt.h"

#define HIST_FILE ".stu_history"

static EditLine *__editor = NULL;
static History *__history = NULL;
static HistEvent __hist_event;
static int __cont = 0;
static char *__form = NULL;
static char *__save_path = NULL;

static char
*stu_prompt(EditLine *el)
{
    return __cont ? "> " : "stu> ";
}

extern void
Prompt_init(const char *progname)
{
    if ((__editor = el_init(progname, stdin, stdout, stderr)) == NULL)
        errx(1, "could not init editline");
    el_set(__editor, EL_PROMPT, &stu_prompt);
    el_set(__editor, EL_EDITOR, "emacs");

    if ((__history = history_init()) == NULL)
        errx(1, "could not init editline history");
    history(__history, &__hist_event, H_SETSIZE, 800);
    el_set(__editor, EL_HIST, history, __history);

    /* Try and load history from a file. */
    char *home = getenv("HOME");
    if (!home)
        home = "";

    __save_path = calloc(
        strlen(home) + strlen("/") + strlen(HIST_FILE) + 1, sizeof(char));
    if (__save_path == NULL)
        err(1, "history path");
    strcat(__save_path, home);
    strcat(__save_path, "/");
    strcat(__save_path, HIST_FILE);

    history(__history, &__hist_event, H_LOAD, __save_path);
}

extern int
Prompt_readline(char **form)
{
    int count = 0, total = 0, size;
    char const *line = NULL;
    char *partial;

    *form = NULL;
    do {
        if ((line = el_gets(__editor, &count)) != NULL) {
            total += count;
            partial = strdup(line);
            if (*form == NULL) {
                *form = partial;
            } else {
                size = strlen(*form) + strlen(partial) + 1;
                *form = realloc(*form, size);
                strcat(*form, partial);
                free(partial);
            }

            if (Stu_is_valid_form(NULL, *form) < 0) {
                __cont = 1;
            } else {
                break;
            }
        } else if (!__cont) {
            free(*form);
            *form = NULL;
            break;
        } else {
            break;
        }
    } while (__cont);

    /* Explicitly reset continuation flag. */
    __cont = 0;

    return total;
}

extern void
Prompt_save(const char *to_save)
{
    history(__history, &__hist_event, H_ENTER, to_save);
}

extern void
Prompt_finish(void)
{
    /* Flush history. */
    history(__history, &__hist_event, H_SAVE, __save_path);
    free(__save_path);
    __save_path = NULL;
    history_end(__history);
    el_end(__editor);
}
