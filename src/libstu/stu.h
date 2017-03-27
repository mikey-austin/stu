#ifndef STU_DEFINED
#define STU_DEFINED

#include "gc.h"
#include "sv.h"
#include "svlist.h"
#include "hash.h"

/* Private structure stub definitions. */
struct Scope;
struct Env;

/* Global nil object. */
extern Sv *Sv_nil;
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
    Gc *gc_head;
    Gc *gc_tail;
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
    Hash *sym_name_to_id;
    char **sym_id_to_name;
    long sym_num_ids;
} Stu;

extern Stu *Stu_new(void);
extern void Stu_destroy(Stu **);
extern Svlist *Stu_parse_buf(Stu *, const char *);
extern Svlist *Stu_parse_file(Stu *, const char *);

#endif
