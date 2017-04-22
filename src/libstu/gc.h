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

#ifndef GC_DEFINED
#define GC_DEFINED

#include <stdio.h>

#define GC_MARK_MASK  0x01
#define GC_LOCK_MASK  0x02
#define GC_TYPE_MASK  0xF0
#define GC_TYPE_BITS  4
#define GC_TYPE_SV    0x01
#define GC_TYPE_ENV   0x02

#define PUSH_SCOPE(s)     Gc_scope_push((s))
#define POP_SCOPE(s)      Gc_scope_pop((s))
#define POP_N_SAVE(s, x)  POP_SCOPE((s)); Gc_scope_save((s), (Gc *) (x))
#define PUSH_N_SAVE(s, x) PUSH_SCOPE((s)); Gc_scope_save((s), (Gc *) (x))
#define GC_SWEEPABLE(x)   ((x) ? !GC_MARKED((x)) && !GC_LOCKED((x)) : 0)
#define GC_MARKED(x)      ((x) ? (((Gc *) x)->flags & GC_MARK_MASK) : 0)
#define GC_MARK(x)        ((x) ? (((Gc *) x)->flags |= GC_MARK_MASK) : 0)
#define GC_UNMARK(x)      ((x) ? (((Gc *) x)->flags &= ~GC_MARK_MASK) : 0)
#define GC_LOCKED(x)      ((x) ? (((Gc *) x)->flags & GC_LOCK_MASK) : 0)
#define GC_LOCK(x)        ((x) ? (((Gc *) x)->flags |= GC_LOCK_MASK) : 0)
#define GC_UNLOCK(x)      ((x) ? (((Gc *) x)->flags &= ~GC_LOCK_MASK) : 0)
#define GC_PREV(x)        ((x) ? (((Gc *) x)->prev : NULL))
#define GC_NEXT(x)        ((x) ? (((Gc *) x)->next : NULL))
#define GC_IS_SV(x)       ((x) ? (((Gc *) x)->flags >> GC_TYPE_BITS) == GC_TYPE_SV : 0)
#define GC_IS_ENV(x)      ((x) ? (((Gc *) x)->flags >> GC_TYPE_BITS) == GC_TYPE_ENV : 0)
#define GC_INIT(s, x, t)  ((x) ? (((Gc *) x)->flags = (t << GC_TYPE_BITS)) : 0); \
                              Gc_add(s, ((Gc *) x)); \
                              Gc_collect(s)

/* Forward declarations. */
struct Stu;

struct Gc;
typedef struct Gc {
    struct Gc *next;
    struct Gc *prev;
    unsigned char flags; /* Contains "mark" & object type. */
} Gc;

struct Scope;
typedef struct Scope {
    struct Scope *prev;
    Gc *val;
} Scope;

extern void Gc_collect(struct Stu *);
extern void Gc_add(struct Stu *, Gc *);
extern void Gc_del(struct Stu *, Gc *);
extern void Gc_lock(struct Stu *, Gc *);
extern void Gc_unlock(struct Stu *, Gc *);
extern void Gc_mark(struct Stu *, Gc *);
extern void Gc_sweep(struct Stu *, int);
extern void Gc_scope_push(struct Stu *);
extern void Gc_scope_pop(struct Stu *);
extern void Gc_scope_save(struct Stu *, Gc *);
extern void Gc_dump_stats(struct Stu *, FILE *);

#endif
