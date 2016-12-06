#include <stdio.h>
#include <stdlib.h>

#include "gc.h"
#include "sv.h"
#include "env.h"

/* Entry points of global gc-managed structure list. */
static Gc *Gc_head = NULL;
static Gc *Gc_tail = NULL;
static int Gc_allocs = 0;

static int Stats_managed_objects = 0;
static int Stats_collections = 0;
static int Stats_frees = 0;
static int Stats_allocs = 0;
static int Stats_cleaned = 0;

extern void
Gc_collect(void)
{
    int before_collect = Stats_managed_objects;

    if (Gc_allocs > GC_THRESHOLD) {
        Gc_mark((Gc *) MAIN_ENV);
        Gc_sweep(1);
        Gc_allocs = 0;
        Stats_cleaned += before_collect - Stats_managed_objects;
        Stats_collections++;
    }
}

extern void
Gc_add(Gc *gc)
{
    Stats_managed_objects++;
    Stats_allocs++;
    Gc_allocs++;
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
    Stats_managed_objects--;
    Stats_frees++;
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
    for (; env; env = env->prev) {
        Gc_mark((Gc *) env);
        Gc_mark((Gc *) env->val);
    }
}

extern void
Gc_mark(Gc *gc)
{
    if (gc && !GC_MARKED(gc)) {
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
        next = cur->prev;
        if (!only_unmarked || (only_unmarked && !GC_MARKED(cur))) {
            Gc_del(cur);
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
        } else {
            GC_UNMARK(cur);
        }
        cur = next;
    }
}

extern void
Gc_dump_stats(void)
{
    fprintf(stderr, "--\n");
    fprintf(stderr, "Avg cleanups per gc: %.2f\n", Stats_cleaned / (Stats_collections + 1.0));
    fprintf(stderr, "Number of gc:        %d\n", Stats_collections);
    fprintf(stderr, "Number of allocs:    %d\n", Stats_allocs);
    fprintf(stderr, "Number of frees:     %d\n", Stats_frees);
}
