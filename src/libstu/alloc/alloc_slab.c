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
#include <string.h>
#include <err.h>
#include <limits.h>

#include "config.h"
#include "alloc.h"
#include "alloc_slab.h"

#define INITIAL_NUM_SLABS 5

typedef struct Slab {
    int *free_list;
    void *chunks;
    int free_list_head;
    int chunk_head;
} Slab;

/* Slab allocator child class. */
typedef struct AllocSlab {
    Alloc base;
    Slab *slabs;
    int num_slabs;
    int used_slabs;
    int slab_size;
} AllocSlab;

static void
init_slab(AllocSlab *allocator, Slab *slab)
{
    slab->chunk_head = -1;
    slab->chunks = malloc(
        allocator->slab_size * allocator->base.size);
    if (slab->chunks == NULL)
        err(1, "init_slab; could not allocate slab chunks");

    slab->free_list_head = -1;
    slab->free_list = malloc(
        allocator->slab_size * sizeof(*slab->free_list));
    if (slab->free_list == NULL)
        err(1, "init_slab; could not allocate free list");
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
    init_slab(new, &new->slabs[new->used_slabs++]);

    return (Alloc *) new;
}

static void
free_slab(Slab *slab)
{
    free(slab->chunks);
    free(slab->free_list);
}

extern void
AllocSlab_destroy(Alloc *base)
{
    AllocSlab *allocator = (AllocSlab *) base;
    int i;

    for (i = 0; i < allocator->used_slabs; i++)
        free_slab(&allocator->slabs[i]);

    free(allocator->slabs);
}

static Slab
*next_slab(AllocSlab *allocator)
{
    Slab *slab = NULL;

    if (allocator->used_slabs == allocator->num_slabs) {
        allocator->num_slabs *= 2;
        allocator->slabs = realloc(allocator->slabs,
            allocator->num_slabs * sizeof(*(allocator->slabs)));
        if (allocator->slabs == NULL)
            err(1, "next_slab; could not re-allocate slabs");
    }

    slab = &allocator->slabs[allocator->used_slabs++];
    init_slab(allocator, slab);

    return slab;
}

static void
*next_chunk(AllocSlab *allocator, Slab *slab)
{
    int i;
    unsigned long start = (unsigned long) slab->chunks;
    void *chunk = NULL;

    if (slab->free_list_head >= 0) {
        /* From the free list. */
        i = slab->free_list[slab->free_list_head--];
        chunk = (void *) (start + (i * allocator->base.size));
    } else if (slab->chunk_head < (allocator->slab_size - 1)) {
        /* From the unallocated chunks. */
        slab->chunk_head += 1;
        chunk = (void *) (start + (slab->chunk_head * allocator->base.size));
    }

    if (chunk)
        memset(chunk, 0, allocator->base.size);

    return chunk;
}

extern void
*AllocSlab_allocate(Alloc *base)
{
    AllocSlab *allocator = (AllocSlab *) base;
    void *chunk = NULL;
    Slab *slab = NULL;
    int i;

    for (i = 0; i < allocator->used_slabs; i++) {
        if ((chunk = next_chunk(allocator, &allocator->slabs[i])) != NULL)
            return chunk;
    }

    /* No available chunks in allocated slabs, make a new slab. */
    slab = next_slab(allocator);

    return next_chunk(allocator, slab);
}

static int
in_slab(AllocSlab *allocator, Slab *slab, void *chunk)
{
    unsigned long start, end, query;

    query = (unsigned long) chunk;
    start = (unsigned long) slab->chunks;
    end = start + (allocator->slab_size - 1) * allocator->base.size;

    return query >= start && query <= end;
}

static Slab
*find_slab(AllocSlab *allocator, void *chunk)
{
    int i;

    for (i = 0; i < allocator->used_slabs; i++) {
        if (in_slab(allocator, &allocator->slabs[i], chunk))
            return &allocator->slabs[i];
    }

    return NULL;
}

static void
release_chunk(AllocSlab *allocator, Slab *slab, void *chunk)
{
    int i = (((unsigned long) chunk) - ((unsigned long) slab->chunks))
        / allocator->base.size;

    if (i > 0
        && i < allocator->slab_size
        && slab->free_list_head < (allocator->slab_size - 1))
    {
        /* Save the index of the freed chunk. */
        slab->free_list[++slab->free_list_head] = i;
    }
}

extern void
AllocSlab_release(Alloc *base, void *to_release)
{
    AllocSlab *allocator = (AllocSlab *) base;
    Slab *slab = NULL;

    if ((slab = find_slab(allocator, to_release)) != NULL) {
        release_chunk(allocator, slab, to_release);
    } else {
        warnx("Chunk 0x%lx not found in any slabs, ignoring",
              (unsigned long) to_release);
    }
}
