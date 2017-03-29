#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <stdio.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "stu.h"
#include "svlist.h"

extern Svlist
*Svlist_new(void)
{
    Svlist *x = NULL;

    if ((x = calloc(1, sizeof(*x))) == NULL)
        err(1, "Svlist_new");

    return x;
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

    if ((x = calloc(1, sizeof(*x))) == NULL)
        err(1, "Svlist_push");

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
