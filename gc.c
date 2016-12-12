#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "gc.h"
#include "sv.h"
#include "env.h"
#include "hash.h"
#include "symtab.h"

struct Scope;
typedef struct Scope {
    struct Scope *prev;
    Gc *val;
} Scope;

static Scope **scope_stack = NULL;
static int stack_size = 0;
static int max_stack_size = 20;

/* Entry points of global gc-managed structure list. */
static Gc *Gc_head = NULL;
static Gc *Gc_tail = NULL;
static int Gc_allocs = 0;

/* Visualization declarations. */
static Hash *Gc_graphviz_nodes = NULL;
static void Gc_dump_sv_graphviz(FILE *, Sv *);
static void Gc_dump_env_graphviz(FILE *, Env *);

static int Stats_managed_objects = 0;
static int Stats_collections = 0;
static int Stats_frees = 0;
static int Stats_allocs = 0;
static int Stats_cleaned = 0;
static int Scope_pushes = 0;
static int Scope_pops = 0;

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

static void
Gc_mark_scope(Scope *scope) {
    for (; scope; scope = scope->prev)
        Gc_mark(scope->val);
}

extern void
Gc_collect(void)
{
    int before_collect = Stats_managed_objects;

    if (Gc_allocs > GC_THRESHOLD) {
        Gc_mark((Gc *) MAIN_ENV);
        for (int i = 0; i < stack_size; i++)
            Gc_mark_scope(scope_stack[i]);
        Gc_sweep(1);
        Gc_allocs = 0;
        Stats_cleaned += (before_collect - Stats_managed_objects);
        Stats_collections++;
    }
}

static Scope
*Gc_new_scope()
{
    Scope *new = NULL;

    if ((new = calloc(1, sizeof(*new))) == NULL)
        err(1, "Gc_new_scope");

    return new;
}

extern void
Gc_scope_push(void)
{
    Scope *new = Gc_new_scope();

    if (!scope_stack || (stack_size + 1) > max_stack_size) {
        if (scope_stack)
            max_stack_size *= 2;
        scope_stack = realloc(
            scope_stack, max_stack_size * sizeof(*scope_stack));
        if (scope_stack == NULL)
            err(1, "Gc_scope_push");
    }

    scope_stack[stack_size++] = new;
    Scope_pushes += 1;
}

extern void
Gc_scope_pop(void)
{
    Scope *old = scope_stack[stack_size - 1], *prev = NULL;
    scope_stack[stack_size - 1] = NULL;
    stack_size -= 1;

    while (old) {
        prev = old->prev;
        free(old);
        old = prev;
    }

    if (stack_size == 0) {
        free(scope_stack);
        scope_stack = NULL;
    }

    Scope_pops += 1;
}

/* Save result in the top scope if it exists. */
extern
void Gc_scope_save(Gc *gc)
{
    Scope *top = stack_size > 0 ? scope_stack[stack_size - 1] : NULL, *new;
    if (top) {
        new = Gc_new_scope();
        new->prev = top;
        new->val = gc;
        scope_stack[stack_size - 1] = new;
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

    Gc_scope_save(gc);
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
    fprintf(stderr, "Avg cleanups per gc: %.2f (%d cleaned)\n",
            Stats_cleaned / (Stats_collections + 1.0), Stats_cleaned);
    fprintf(stderr, "Number of gcs:       %d\n", Stats_collections);
    fprintf(stderr, "Number of allocs:    %d\n", Stats_allocs);
    fprintf(stderr, "Number of frees:     %d\n", Stats_frees);
    fprintf(stderr, "Scope pushes:        %d\n", Scope_pushes);
    fprintf(stderr, "Scope pops:          %d\n", Scope_pops);
}

/*
 * Dump visualization of GC internal structures.
 */
static void
Gc_register_graphviz_node(Hash *nodes, Gc *node)
{
    static char key[1000];
    snprintf(key, sizeof(key), "%lx", (unsigned long) node);
    Hash_put(Gc_graphviz_nodes, key, node);
}

static int
Gc_graphviz_node_exists(Hash *nodes, Gc *node)
{
    static char key[1000];
    snprintf(key, sizeof(key), "%lx", (unsigned long) node);
    return Hash_get(Gc_graphviz_nodes, key) != NULL;
}

static void
Gc_dump_sv_graphviz(FILE *out, Sv *sv)
{
    if (sv && !Gc_graphviz_node_exists(Gc_graphviz_nodes, (Gc *) sv)) {
        switch (sv->type) {
        case SV_CONS:
            fprintf(out, "\"%lx\" -> \"%lx\" [color=\"green\"]\n",
                    (unsigned long) sv, (unsigned long) CAR(sv));
            fprintf(out, "\"%lx\" -> \"%lx\" [color=\"green\"]\n",
                    (unsigned long) sv, (unsigned long) CDR(sv));

            Gc_dump_sv_graphviz(out, CAR(sv));
            Gc_dump_sv_graphviz(out, CDR(sv));

            Gc_register_graphviz_node(Gc_graphviz_nodes, (Gc *) CAR(sv));
            Gc_register_graphviz_node(Gc_graphviz_nodes, (Gc *) CDR(sv));
            break;

        case SV_LAMBDA:
            if (sv->val.ufunc) {
                fprintf(out, "\"%lx\" -> \"%lx\" [color=\"red\"]\n",
                        (unsigned long) sv, (unsigned long) sv->val.ufunc->env);
                fprintf(out, "\"%lx\" -> \"%lx\" [color=\"green\"]\n",
                    (unsigned long) sv, (unsigned long) sv->val.ufunc->formals);
                fprintf(out, "\"%lx\" -> \"%lx\" [color=\"green\"]\n",
                    (unsigned long) sv, (unsigned long) sv->val.ufunc->body);

                Gc_dump_env_graphviz(out, sv->val.ufunc->env);
                Gc_dump_sv_graphviz(out, sv->val.ufunc->formals);
                Gc_dump_sv_graphviz(out, sv->val.ufunc->body);

                Gc_register_graphviz_node(Gc_graphviz_nodes, (Gc *) sv->val.ufunc->env);
                Gc_register_graphviz_node(Gc_graphviz_nodes, (Gc *) sv->val.ufunc->formals);
                Gc_register_graphviz_node(Gc_graphviz_nodes, (Gc *) sv->val.ufunc->body);
            }
            break;

        default:
            /* Ignore. */
            break;
        }
    }
}

static void
Gc_dump_env_graphviz(FILE *out, Env *env)
{
    for (; env; env = env->prev) {
        Gc_register_graphviz_node(Gc_graphviz_nodes, (Gc *) env);
        if (env->prev) {
            fprintf(out, "\"%lx\" -> \"%lx\" [color=\"red\"]\n",
                    (unsigned long) env, (unsigned long) env->prev);
        }

        if (env->val) {
            fprintf(out, "\"%lx\" -> \"%lx\" [color=\"green\"]\n",
                    (unsigned long) env, (unsigned long) env->val);
            Gc_dump_sv_graphviz(out, env->val);
        }
    }
}

static void
Gc_dump_graphviz_sv_node(FILE *out, const char *key, Sv *sv)
{
    switch (sv->type) {
    case SV_SYM:
        fprintf(out, "\"%s\" [label=\"%s%s [0x%s]\n%s\",style=\"filled\"]\n",
                key, "Sym", (GC_MARKED((Gc *) sv) ? " (marked)" : ""), key,
                Symtab_get_name(sv->val.i));
        break;

    case SV_STR:
        fprintf(out, "\"%s\" [label=\"%s%s [0x%s]\n\"%s\"\",style=\"filled\"]\n",
                key, "Str", (GC_MARKED((Gc *) sv) ? " (marked)" : ""), key,
                sv->val.buf);
        break;

    case SV_INT:
        fprintf(out, "\"%s\" [label=\"%s%s [0x%s]\n%ld\",style=\"filled\"]\n",
                key, "Int", (GC_MARKED((Gc *) sv) ? " (marked)" : ""), key, sv->val.i);
        break;

    case SV_BOOL:
        fprintf(out, "\"%s\" [label=\"%s%s [0x%s]\n%s\",style=\"filled\"]\n",
                key, "Int", (GC_MARKED((Gc *) sv) ? " (marked)" : ""), key,
                sv->val.i ? "#t" : "#f");
        break;

    case SV_FUNC:
        fprintf(out, "\"%s\" [label=\"%s%s [0x%s]\",style=\"filled\"]\n",
                key, "Builtin", (GC_MARKED((Gc *) sv) ? " (marked)" : ""), key);
        break;

    case SV_LAMBDA:
        fprintf(out, "\"%s\" [label=\"%s%s [0x%s]\",style=\"filled\"]\n",
                key, "Lambda", (GC_MARKED((Gc *) sv) ? " (marked)" : ""), key);
        break;

    case SV_NIL:
        fprintf(out, "\"%s\" [label=\"%s%s [0x%s]\",style=\"filled\"]\n",
                key, "Nil", (GC_MARKED((Gc *) sv) ? " (marked)" : ""), key);
        break;

    default:
        /* Ignore. */
        break;
    }
}

static void
Gc_dump_graphviz_node(FILE *out, const char *key, Gc *gc)
{
    switch (gc->flags >> GC_TYPE_BITS) {
    case GC_TYPE_SV:
        Gc_dump_graphviz_sv_node(out, key, (Sv *) gc);
        break;

    case GC_TYPE_ENV:
        fprintf(out, "\"%s\" [shape=\"box\",label=\"%s%s [0x%s]\n%s\"]\n",
                key, "Env", (GC_MARKED(gc) ? " (marked)" : ""), key,
                Symtab_get_name(((Env *) gc)->sym));
        break;
    }
}

extern void
Gc_dump_graphviz(FILE *out)
{
    Gc *cur = Gc_head;

    if (!out)
        out = stderr;

    if (Gc_graphviz_nodes)
        Hash_destroy(&Gc_graphviz_nodes);
    Gc_graphviz_nodes = Hash_new(NULL);

    fprintf(out, "digraph gc {\n");

    /* Dump every gc managed object. */
    while (cur && cur->prev) {
        Gc_register_graphviz_node(Gc_graphviz_nodes, cur);
        fprintf(out, "\"%lx\" -> \"%lx\" [color=\"blue\"]\n",
                (unsigned long) cur, (unsigned long) cur->prev);
        cur = cur->prev;
    }

    /* Crawl environments. */
    Gc_dump_env_graphviz(out, MAIN_ENV);

    /* Print detailed node information. */
    Hash_ent *ent = Hash_entries(Gc_graphviz_nodes);
    for (; ent; ent = NEXT_ENTRY(ent)) {
        Gc_dump_graphviz_node(out, ent->k, (Gc *) ent->v);
    }

    fprintf(out, "}\n");

    Hash_destroy(&Gc_graphviz_nodes);
}

extern void
Gc_dump_graphviz_file(const char *filename)
{
    FILE *handle = fopen(filename, "w+");
    if (handle == NULL)
        err(1, "Gc_dump_graphviz_file");
    Gc_dump_graphviz(handle);
    fclose(handle);
}
