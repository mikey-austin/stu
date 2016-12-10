#ifndef GC_DEFINED
#define GC_DEFINED

#define PUSH_SCOPE   Gc_scope_push()
#define POP_SCOPE    Gc_scope_pop()
#define GC_MARK_MASK 0x01
#define GC_TYPE_MASK 0xF0
#define GC_TYPE_BITS 4
#define GC_TYPE_SV   0x01
#define GC_TYPE_ENV  0x02
#define GC_THRESHOLD  2

#define GC_MARKED(x)  ((x) ? (((Gc *) x)->flags & GC_MARK_MASK) : 0)
#define GC_MARK(x)    ((x) ? (((Gc *) x)->flags |= GC_MARK_MASK) : 0)
#define GC_UNMARK(x)  ((x) ? (((Gc *) x)->flags &= ~GC_MARK_MASK) : 0)
#define GC_PREV(x)    ((x) ? (((Gc *) x)->prev : NULL))
#define GC_NEXT(x)    ((x) ? (((Gc *) x)->next : NULL))
#define GC_IS_SV(x)   ((x) ? (((Gc *) x)->flags >> GC_TYPE_BITS) == GC_TYPE_SV : 0)
#define GC_IS_ENV(x)  ((x) ? (((Gc *) x)->flags >> GC_TYPE_BITS) == GC_TYPE_ENV : 0)
#define GC_INIT(x, t) ((x) ? (((Gc *) x)->flags = (t << GC_TYPE_BITS)) : 0); Gc_add(((Gc *) x))

/* Forward declarations. */
struct Gc;

typedef struct Gc {
    struct Gc *next;
    struct Gc *prev;
    unsigned char flags; /* Contains "mark" & object type. */
} Gc;

extern void Gc_dump_stats(void);
extern void Gc_collect(void);
extern void Gc_add(Gc *);
extern void Gc_del(Gc *);
extern void Gc_mark(Gc *);
extern void Gc_sweep(int);
extern void Gc_scope_push(void);
extern void Gc_scope_pop(void);

#endif
