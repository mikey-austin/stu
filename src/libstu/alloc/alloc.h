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

#ifndef ALLOC_DEFINED
#define ALLOC_DEFINED

#include <stddef.h>

enum Alloc_type {
    ALLOC_TYPE_SYS,
    ALLOC_TYPE_SLAB
};

typedef struct Stu Stu;
typedef struct Alloc Alloc;

struct Alloc {
    enum Alloc_type type;
    Stu *stu;
    size_t size;
    void (*destroy)(Alloc *);
    void *(*allocate)(Alloc *);
    void (*release)(Alloc *, void *);
};

extern Alloc *Alloc_new(Stu *, size_t, enum Alloc_type);
extern void Alloc_destroy(Alloc **);
extern void *Alloc_allocate(Alloc *);
extern void Alloc_release(Alloc *, void *);

#endif
