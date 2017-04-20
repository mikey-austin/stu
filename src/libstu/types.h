/*
 * Copyright (c) 2017 Raphael Sousa Santos <contact@raphaelss.com>
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

#ifndef TYPES_DEFINED
#define TYPES_DEFINED

/* Forward declarations */
typedef struct Stu Stu;
typedef struct Sv Sv;

typedef struct Type_registry {
    long *name;
    unsigned *arity;
    Sv **value;
    unsigned size, capacity;
} Type_registry;

typedef struct Type {
    unsigned i;
} Type;

extern void Type_registry_init(Type_registry *);
extern void Type_registry_release(Type_registry *);
extern Type Type_new(Stu *, long, unsigned);
extern long Type_name(Stu *, Type);
extern unsigned Type_arity(Stu *, Type);
extern Sv *Type_value(Stu *, Type);

#endif
