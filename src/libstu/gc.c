/*
 * Copyright (c) 2016, 2017 Mikey Austin <mikey@jackiemclean.net>
 * Copyright (c) 2017 Raphael Sousa Santos <contact@raphaelss.com>
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

#include "config.h"
#include "alloc/alloc.h"
#include "env.h"
#include "gc.h"
#include "hash.h"
#include "stu_private.h"
#include "sv.h"
#include "symtab.h"
#include "utils.h"

static void
Gc_visit_sv(Stu *stu, Sv *sv, void (*action)(Stu *, Gc *))
{
    if (sv) {
        switch (sv->type) {
        case SV_CONS:
            action(stu, (Gc *) CAR(sv));
            action(stu, (Gc *) CDR(sv));
            break;

        case SV_SPECIAL:
            if (sv->val.special) {
                action(stu, (Gc *) sv->val.special->body);
            }
            break;

        case SV_TUPLE:
            for (unsigned i = 0; i < Type_arity(stu, sv->val.tuple->type); i++) {
                action(stu, (Gc *) sv->val.tuple->values[i]);
            }
            break;

        case SV_LAMBDA:
            if (sv->val.ufunc) {
                action(stu, (Gc *) sv->val.ufunc->env);
                action(stu, (Gc *) sv->val.ufunc->formals);
                action(stu, (Gc *) sv->val.ufunc->body);
            }
            break;

        default:
            /* Already marked. */
            break;
        }
    }
}

static void
Gc_visit_env(Stu *stu, Env *env, void (*action)(Stu *, Gc *))
{
    for (; env; env = env->prev) {
        action(stu, (Gc *) env);
        action(stu, (Gc *) env->val);
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
        for (Scope *scope = stu->gc_scope_stack; scope; scope = scope->stack_prev)
            Gc_mark_scope(stu, scope);
        Gc_sweep(stu, 0);
        stu->gc_allocs = 0;
        stu->stats_gc_cleaned += (before_collect - stu->stats_gc_managed_objects);
        stu->stats_gc_collections++;
    }
}

extern void
Gc_scope_push(Stu *stu)
{
    Scope *new = Alloc_allocate(stu->gc_scope_alloc);
    new->stack_prev = stu->gc_scope_stack;
    stu->gc_scope_stack = new;
    stu->stats_gc_scope_pushes += 1;
}

extern void
Gc_scope_pop(Stu *stu)
{
    Scope *old = stu->gc_scope_stack, *prev = NULL;

    if (old != NULL)
        stu->gc_scope_stack = old->stack_prev;
    else
        return;

    while (old) {
        prev = old->prev;
        Alloc_release(stu->gc_scope_alloc, old);
        old = prev;
    }

    stu->stats_gc_scope_pops += 1;
}

/* Save result in the top scope if it exists. */
extern
void Gc_scope_save(Stu *stu, Gc *gc)
{
    Scope *new, *top = stu->gc_scope_stack;

    if (top) {
        new = Alloc_allocate(stu->gc_scope_alloc);
        new->prev = top->prev;
        top->prev = new;
        new->val = gc;
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
            Gc_visit_sv(stu, (Sv *) gc, Gc_mark);
            break;

        case GC_TYPE_ENV:
            Gc_visit_env(stu, (Env *) gc, Gc_mark);
            break;
        }
    }
}

extern void
Gc_lock(Stu *stu, Gc *gc)
{
    if (gc && !GC_LOCKED(gc)) {
        GC_LOCK(gc);
        switch (gc->flags >> GC_TYPE_BITS) {
        case GC_TYPE_SV:
            Gc_visit_sv(stu, (Sv *) gc, Gc_lock);
            break;

        case GC_TYPE_ENV:
            Gc_visit_env(stu, (Env *) gc, Gc_lock);
            break;
        }
    }
}

extern void
Gc_unlock(Stu *stu, Gc *gc)
{
    if (gc && !GC_LOCKED(gc)) {
        GC_UNLOCK(gc);
        switch (gc->flags >> GC_TYPE_BITS) {
        case GC_TYPE_SV:
            Gc_visit_sv(stu, (Sv *) gc, Gc_unlock);
            break;

        case GC_TYPE_ENV:
            Gc_visit_env(stu, (Env *) gc, Gc_unlock);
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
