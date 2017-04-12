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

#ifndef STU_PRIVATE_DEFINED
#define STU_PRIVATE_DEFINED

struct Svlist;
struct Scope;
struct Env;
struct Hash;
struct Sv;

/*
 * Main stu interpreter structure.
 */
typedef struct Stu {
    /* GC structures. */
    struct Scope **gc_scope_stack;
    int gc_stack_size;
    int max_gc_stack_size;

    /* Entry points of interpreter gc-managed structure list. */
    struct Gc *gc_head;
    struct Gc *gc_tail;
    int gc_allocs;

    /* GC stats. */
    int stats_gc_managed_objects;
    int stats_gc_collections;
    int stats_gc_frees;
    int stats_gc_allocs;
    int stats_gc_cleaned;
    int stats_gc_scope_pushes;
    int stats_gc_scope_pops;

    /* Main environment. */
    struct Env *main_env;

    /* Symbol table structures. */
    struct Hash *sym_name_to_id;
    char **sym_id_to_name;
    long sym_num_ids;
} Stu;

extern struct Sv *Sv_nil;
#define NIL Sv_nil

/* Internal helper functions. */
extern struct Svlist *Stu_parse_buf(Stu *, const char *);
extern struct Svlist *Stu_parse_file(Stu *, const char *);

#endif
