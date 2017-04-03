#ifndef STU_DEFINED
#define STU_DEFINED

#include <stdio.h>

/* Private structure stub definitions. */
struct Svlist;
struct Scope;
struct Env;
struct Hash;
typedef struct Sv Sv;

/* Global nil object. */
extern struct Sv *Sv_nil;
#define NIL Sv_nil

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

extern Stu *Stu_new(void);
extern void Stu_destroy(Stu **);
extern struct Svlist *Stu_parse_buf(Stu *, const char *);
extern struct Svlist *Stu_parse_file(Stu *, const char *);
extern struct Sv *Stu_eval_file(Stu *, const char *);
extern struct Sv *Stu_eval_buf(Stu *, const char *);
extern void Stu_dump_sv(Stu *, struct Sv *, FILE *);
extern void Stu_dump_stats(Stu *, FILE *);

#endif
