/*
 * Copyright (c) 2017 Mikey Austin <mikey@jackiemclean.net>
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

#include <stdlib.h>
#include <err.h>
#include <limits.h>

#include "config.h"
#include "alloc.h"
#include "alloc_slab.h"

#define INITIAL_NUM_SLABS 5

typedef struct Slab {
    unsigned short *free_list;
    void *entries;
    unsigned short num_released;
} Slab;

/* Slab allocator child class. */
typedef struct AllocSlab {
    Alloc base;
    Slab **slabs;
    int num_slabs;
    int used_slabs;
    unsigned short slab_size;
} AllocSlab;

static void
init_slab(AllocSlab *allocator, Slab *slab)
{
    slab->entries = calloc(
        allocator->slab_size, allocator->base.size);
    if (slab->entries == NULL)
        err(1, "init_slab; could not allocate slab entries");

    slab->free_list = calloc(
        allocator->slab_size, sizeof(*slab->free_list));
    if (slab->free_list == NULL)
        err(1, "init_slab; could not allocate free list");
}

static void
free_slab(Slab *slab)
{
    free(slab->entries);
    free(slab->free_list);
}

extern Alloc
*AllocSlab_new(Alloc base)
{
    AllocSlab *new = NULL;

    if ((new = calloc(1, sizeof(*new))) == NULL)
        err(1, "AllocSlab_new");

    new->base = base;
    new->base.allocate = AllocSlab_allocate;
    new->base.release = AllocSlab_release;
    new->base.destroy = AllocSlab_destroy;

    new->used_slabs = 0;
    new->num_slabs = INITIAL_NUM_SLABS;
    new->slab_size = SLAB_SIZE;
    new->slabs = calloc(new->num_slabs, sizeof(*(new->slabs)));
    if (new->slabs == NULL)
        err(1, "AllocSlab_new; could not allocate slabs");

    /* Create a slab up front. */
    init_slab(new, new->slabs[new->used_slabs++]);

    return (Alloc *) new;
}

extern void
AllocSlab_destroy(Alloc *base)
{
    int i;
    AllocSlab *allocator = (AllocSlab *) base;

    for (i = 0; i < allocator->used_slabs; i++)
        free_slab(allocator->slabs[i]);

    free(allocator->slabs);
}

extern void
*AllocSlab_allocate(Alloc *allocator)
{
    // TODO
}

extern void
AllocSlab_release(Alloc *allocator, void *to_release)
{
    // TODO
}
