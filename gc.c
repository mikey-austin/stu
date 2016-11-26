#include <stdio.h>
#include <stdlib.h>

#include "gc.h"
#include "sv.h"
#include "env.h"

/* Entry points of global gc-managed structure list. */
static Gc *Gc_head = NULL;
static Gc *Gc_tail = NULL;

extern void
Gc_add(Gc *gc)
{
    if (Gc_head == NULL) {
        Gc_head = Gc_tail = gc;
    } else {
        /* Add to the end of the list. */
        gc->next = Gc_tail;
        Gc_tail->prev = gc;
        Gc_tail = gc;
    }
}

extern void
Gc_del(Gc *gc)
{
    if (gc->prev == NULL)
        Gc_tail = gc->next;
    else
        gc->prev->next = gc->next;

    if (gc->next == NULL)
        Gc_head = gc->prev;
    else
        gc->next->prev = gc->prev;
}

static void
Gc_mark_sv(Sv *sv) {
    if (sv) {
        switch (sv->type) {
        case SV_CONS:
            Gc_mark((Gc *) CAR(sv));
            Gc_mark((Gc *) CDR(sv));
            break;

        case SV_LAMBDA:
            if (sv->val.ufunc) {
                Gc_mark((Gc *) sv->val.ufunc->env);
                Gc_mark((Gc *) sv->val.ufunc->formals);
                Gc_mark((Gc *) sv->val.ufunc->body);
            }
            break;

        default:
            /* Already marked. */
            break;
        }
    }
}

static void
Gc_mark_env(Env *env) {
    Sv **contents = NULL;
    int num_contents = 0, i;

    if (env && (num_contents = Env_contents(env, &contents)) > 0) {
        for (i = 0; i < num_contents; i++)
            Gc_mark((Gc *) contents[i]);
        free(contents);
    }
}

extern void
Gc_mark(Gc *gc)
{
    if (gc) {
        GC_MARK(gc);
        switch (gc->flags >> GC_TYPE_BITS) {
        case GC_TYPE_SV:
            Gc_mark_sv((Sv *) gc);
            break;

        case GC_TYPE_ENV:
            Gc_mark_env((Env *) gc);
            break;
        }
    }
}

extern void
Gc_sweep(int only_unmarked)
{
    Sv *sv = NULL;
    Env *env = NULL;
    Gc *cur = Gc_head, *next = NULL;

    while (cur) {
        Gc_del(cur);
        next = cur->prev;
        if (!only_unmarked || (only_unmarked && !GC_MARKED(cur))) {
            switch (cur->flags >> GC_TYPE_BITS) {
            case GC_TYPE_SV:
                sv = (Sv *) cur;
                Sv_destroy(&sv);
                break;

            case GC_TYPE_ENV:
                env = (Env *) cur;
                Env_destroy(&env);
                break;
            }
        }
        cur = next;
    }
}
