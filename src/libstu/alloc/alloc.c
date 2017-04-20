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

#include "config.h"
#include "alloc.h"
#include "alloc_sys.h"
#include "alloc_slab.h"

extern Alloc
*Alloc_new(Stu *stu, size_t size, enum Alloc_type type)
{
    Alloc *new = NULL, base;

    memset(&base, 0, sizeof(base));
    base.type = type;
    base.stu = stu;
    base.size = size;

    switch (type) {
    case ALLOC_TYPE_SYS:
        new = AllocSys_new(base);
        break;

    case ALLOC_TYPE_SLAB:
        new = AllocSlab_new(base);
        break;

    default:
        err(1, "Unknown allocator requested");
    }

    return new;
}

extern void
Alloc_destroy(Alloc **to_destroy)
{
    Alloc *allocator = NULL;

    if (to_destroy == NULL || (allocator = *to_destroy) == NULL)
        return;

    if (allocator->destroy != NULL)
        allocator->destroy(allocator);

    free(allocator);
    *to_destroy = NULL;
}

extern void
*Alloc_allocate(Alloc *allocator)
{
    return allocator->allocate(allocator);
}

extern void
Alloc_release(Alloc *allocator, void *to_release)
{
    allocator->release(allocator, to_release);
}
