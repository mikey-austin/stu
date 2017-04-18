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
#include <err.h>

#include "env.h"
#include "gc.h"
#include "hash.h"
#include "stu_private.h"
#include "sv.h"
#include "symtab.h"
#include "utils.h"

/*
 * Private scope structure to protect unregistered gc objects
 * from premature collection.
 */
struct Scope;
typedef struct Scope {
    struct Scope *prev;
    Gc *val;
} Scope;

static void
Gc_mark_sv(Stu *stu, Sv *sv)
{
    if (sv) {
        switch (sv->type) {
        case SV_CONS:
            Gc_mark(stu, (Gc *) CAR(sv));
            Gc_mark(stu, (Gc *) CDR(sv));
            break;

        case SV_SPECIAL:
            if (sv->val.special) {
                Gc_mark(stu, (Gc *) sv->val.special->body);
            }
            break;

        case SV_LAMBDA:
            if (sv->val.ufunc) {
                Gc_mark(stu, (Gc *) sv->val.ufunc->env);
                Gc_mark(stu, (Gc *) sv->val.ufunc->formals);
                Gc_mark(stu, (Gc *) sv->val.ufunc->body);
            }
            break;

        default:
            /* Already marked. */
            break;
        }
    }
}

static void
Gc_mark_env(Stu *stu, Env *env)
{
    for (; env; env = env->prev) {
        Gc_mark(stu, (Gc *) env);
        Gc_mark(stu, (Gc *) env->val);
    }
}

static void
Gc_mark_scope(Stu *stu, Scope *scope)
{
    for (; scope; scope = scope->prev)
        Gc_mark(stu, scope->val);
}

extern void
Gc_collect(Stu *stu)
{
    int before_collect = stu->stats_gc_managed_objects, i;

    if (stu->gc_allocs > GC_THRESHOLD) {
        Gc_mark(stu, (Gc *) stu->main_env);
        for (i = 0; i < stu->gc_stack_size; i++)
            Gc_mark_scope(stu, stu->gc_scope_stack[i]);
        Gc_sweep(stu, 0);
        stu->gc_allocs = 0;
        stu->stats_gc_cleaned += (before_collect - stu->stats_gc_managed_objects);
        stu->stats_gc_collections++;
    }
}

static Scope
*Gc_new_scope()
{
    return CHECKED_CALLOC(1, sizeof(Scope));
}

extern void
Gc_scope_push(Stu *stu)
{
    Scope *new = Gc_new_scope();

    if (!stu->gc_scope_stack
        || (stu->gc_stack_size + 1) > stu->max_gc_stack_size)
    {
        if (stu->gc_scope_stack)
            stu->max_gc_stack_size *= 2;
        stu->gc_scope_stack = realloc(
            stu->gc_scope_stack,
            stu->max_gc_stack_size * sizeof(*(stu->gc_scope_stack)));
        if (stu->gc_scope_stack == NULL)
            err(1, "Gc_scope_push");
    }

    stu->gc_scope_stack[stu->gc_stack_size++] = new;
    stu->stats_gc_scope_pushes += 1;
}

extern void
Gc_scope_pop(Stu *stu)
{
    Scope *old = stu->gc_scope_stack[stu->gc_stack_size - 1], *prev = NULL;
    stu->gc_scope_stack[stu->gc_stack_size - 1] = NULL;
    stu->gc_stack_size -= 1;

    while (old) {
        prev = old->prev;
        free(old);
        old = prev;
    }

    if (stu->gc_stack_size == 0) {
        free(stu->gc_scope_stack);
        stu->gc_scope_stack = NULL;
    }

    stu->stats_gc_scope_pops += 1;
}

/* Save result in the top scope if it exists. */
extern
void Gc_scope_save(Stu *stu, Gc *gc)
{
    Scope *new, *top = stu->gc_stack_size > 0
        ? stu->gc_scope_stack[stu->gc_stack_size - 1]
        : NULL;

    if (top) {
        new = Gc_new_scope();
        new->prev = top;
        new->val = gc;
        stu->gc_scope_stack[stu->gc_stack_size - 1] = new;
    }
}

extern void
Gc_add(Stu *stu, Gc *gc)
{
    stu->stats_gc_managed_objects++;
    stu->stats_gc_allocs++;
    stu->gc_allocs++;
    if (stu->gc_head == NULL) {
        stu->gc_head = stu->gc_tail = gc;
    } else {
        /* Add to the end of the list. */
        gc->next = stu->gc_tail;
        stu->gc_tail->prev = gc;
        stu->gc_tail = gc;
    }

    Gc_scope_save(stu, gc);
}

extern void
Gc_del(Stu *stu, Gc *gc)
{
    stu->stats_gc_managed_objects--;
    stu->stats_gc_frees++;

    if (gc->prev == NULL)
        stu->gc_tail = gc->next;
    else
        gc->prev->next = gc->next;

    if (gc->next == NULL)
        stu->gc_head = gc->prev;
    else
        gc->next->prev = gc->prev;
}

extern void
Gc_mark(Stu *stu, Gc *gc)
{
    if (gc && !GC_MARKED(gc)) {
        GC_MARK(gc);
        switch (gc->flags >> GC_TYPE_BITS) {
        case GC_TYPE_SV:
            Gc_mark_sv(stu, (Sv *) gc);
            break;

        case GC_TYPE_ENV:
            Gc_mark_env(stu, (Env *) gc);
            break;
        }
    }
}

extern void
Gc_sweep(Stu *stu, int unconditional)
{
    Sv *sv = NULL;
    Env *env = NULL;
    Gc *cur = stu->gc_head, *next = NULL;

    while (cur) {
        next = cur->prev;
        if (unconditional || GC_SWEEPABLE(cur)) {
            Gc_del(stu, cur);
            switch (cur->flags >> GC_TYPE_BITS) {
            case GC_TYPE_SV:
                sv = (Sv *) cur;
                Sv_destroy(stu, &sv);
                break;

            case GC_TYPE_ENV:
                env = (Env *) cur;
                Env_destroy(stu, &env);
                break;
            }
        } else {
            GC_UNMARK(cur);
        }
        cur = next;
    }
}

extern void
Gc_dump_stats(Stu *stu, FILE *out)
{
    fprintf(out, "--\n");
    fprintf(out, "Number of gcs:       %d\n", stu->stats_gc_collections);
    fprintf(out, "Number of allocs:    %d\n", stu->stats_gc_allocs);
    fprintf(out, "Number of frees:     %d\n", stu->stats_gc_frees);
    fprintf(out, "Scope pushes:        %d\n", stu->stats_gc_scope_pushes);
    fprintf(out, "Scope pops:          %d\n", stu->stats_gc_scope_pops);
    fprintf(out, "Avg cleanups per gc: %.2f (%d cleaned)\n",
        stu->stats_gc_cleaned / (stu->stats_gc_collections + 1.0),
        stu->stats_gc_cleaned);
}
