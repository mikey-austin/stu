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
#ifdef ALLOC_SYSTEM
#  include "system.h"
#elif ALLOC_SLAB
#  include "slab.h"
#endif

extern Alloc
*Alloc_new(Stu *stu, size_t size)
{
    Alloc *new = NULL;

    if ((new = calloc(1, sizeof(*new))) == NULL)
        err(1, "Alloc_new");
    new->stu = stu;
    new->size = size;

    return new;
}

extern void
Alloc_destroy(Alloc **to_destroy)
{
    Alloc *alloc = NULL;

    if (to_destroy == NULL || (alloc = *to_destroy) == NULL)
        return;

    free(alloc);
    *to_destroy = NULL;
}

extern void
*Alloc_allocate(Alloc *alloc)
{
#ifdef ALLOC_SYSTEM
    return System_allocate(alloc);
#elif ALLOC_SLAB
    return Slab_allocate(alloc);
#else
#  error "No ALLOCATOR defined!"
#endif
}

extern void
Alloc_release(Alloc *alloc, void *to_release)
{
#ifdef ALLOC_SYSTEM
    System_release(alloc, to_release);
#elif ALLOC_SLAB
    Slab_release(alloc, to_release);
#endif
}
