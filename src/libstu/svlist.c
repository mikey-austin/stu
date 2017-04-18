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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "stu_private.h"
#include "svlist.h"
#include "utils.h"

extern Svlist
*Svlist_new(void)
{
    return CHECKED_CALLOC(1, sizeof(Svlist));
}

extern void
Svlist_destroy(Svlist **list)
{
    Svlist_node *node, *to_delete;

    if (!list || !*list)
        return;

    for (node = (*list)->head; node != NULL; ) {
        to_delete = node;
        node = node->next;
        free(to_delete);
    }

    free(*list);
    *list = NULL;
}

extern void
Svlist_push(Svlist *list, Sv *sv)
{
    Svlist_node *x = NULL;

    if (!list)
        return;

    x = CHECKED_CALLOC(1, sizeof(*x));

    x->sv = sv;
    if (list->tail) {
        list->tail->next = x;
        list->tail = x;
    } else {
        list->head = list->tail = x;
    }

    list->count += 1;
}

extern Sv
*Svlist_eval(Stu *stu, Env *env, Svlist *list)
{
    Svlist_node *cur;
    Sv *result = NULL;

    if (!list || !env)
        return NULL;

    for (cur = list->head; cur; cur = cur->next) {
        /*
         * TODO: add an Env** parameter to Sv_eval in order to propagate
         *       changes to the supplied environment so the caller can keep
         *       track. If the new argument is NULL, do nothing extra.
         *
         *       Replicating previous semantics until we address the above; it
         *       assumes there is only ever one environment.
         */
        result = Sv_eval(stu, env, cur->sv);
        env = stu->main_env;
    }

    return result;
}
