#include <stdio.h>

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

extern void
Gc_destroy_all(void)
{
    Sv *sv = NULL;
    Env *env = NULL;
    Gc *cur = Gc_head, *next = NULL;

    while (cur) {
        Gc_del(cur);
        next = cur->prev;
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
        cur = next;
    }
}
