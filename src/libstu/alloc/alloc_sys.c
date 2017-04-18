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

#include "config.h"
#include "alloc.h"
#include "alloc_sys.h"

extern Alloc
*AllocSys_new(Alloc allocator)
{
    Alloc *new = NULL;

    if ((new = calloc(1, sizeof(*new))) == NULL)
        err(1, "AllocSys_new");

    /*
     * We don't need any special subobject, so just copy onto
     * the heap and return.
     */
    *new = allocator;
    new->allocate = AllocSys_allocate;
    new->release = AllocSys_release;
    new->destroy = NULL;

    return new;
}

extern void
*AllocSys_allocate(Alloc *allocator)
{
    void *block = NULL;

    if ((block = calloc(1, allocator->size)) == NULL)
        err(1, "System_allocate");

    return block;
}

extern void
AllocSys_release(Alloc *allocator, void *to_release)
{
    free(to_release);
}
