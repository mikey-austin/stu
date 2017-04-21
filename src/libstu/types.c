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

#include "stu_private.h"
#include "sv.h"
#include "types.h"
#include "utils.h"

#define INITIAL_CAPACITY 20
#define GROWTH_FACTOR 2

extern void
Type_registry_init(Type_registry *reg)
{
   reg->name = CHECKED_CALLOC(INITIAL_CAPACITY, sizeof(*(reg->name)));
   reg->arity = CHECKED_CALLOC(INITIAL_CAPACITY, sizeof(*(reg->arity)));
   reg->value = CHECKED_CALLOC(INITIAL_CAPACITY, sizeof(*(reg->value)));
   reg->size = 0;
   reg->capacity = INITIAL_CAPACITY;
}

extern void
Type_registry_release(Type_registry *reg)
{
    free(reg->name);
    free(reg->arity);
    free(reg->value);
    reg->name = NULL;
    reg->arity = NULL;
    reg->value = NULL;
}

static void
resize_arrays(Type_registry *reg)
{
    reg->capacity *= GROWTH_FACTOR;
    reg->name = CHECKED_REALLOC(reg->name, reg->capacity * sizeof(*(reg->name)));
    reg->arity = CHECKED_REALLOC(reg->arity, reg->capacity * sizeof(*(reg->arity)));
    reg->value = CHECKED_REALLOC(reg->value, reg->capacity * sizeof(*(reg->value)));
}

extern Type
Type_new(Stu *stu, Sv *name, unsigned arity)
{
    Type t = {0};
    Type_registry *reg = &(stu->type_registry);
    long name_i = name->val.i;

    for (unsigned i = 0; i < reg->size; ++i) {
        if (reg->name[i] == name_i && reg->arity[i] == arity) {
            t.i = i;
            return t;
        }
    }

    if (reg->size == reg->capacity)
        resize_arrays(reg);

    PUSH_SCOPE(stu);
    t.i = reg->size++;
    reg->name[t.i] = name_i;
    reg->arity[t.i] = arity;
    reg->value[t.i] = Sv_new_tuple(stu, Type_new_str(stu, "tuple-type", 2),
        Sv_cons(stu, name, Sv_cons(stu, Sv_new_int(stu, arity), NIL)));
    Gc_lock(stu, (Gc *) reg->value[t.i]);
    POP_SCOPE(stu);

    return t;
}

extern Type
Type_new_str(Stu *stu, const char *str, unsigned arity)
{
    Sv *name = Sv_new_sym(stu, str);
    Gc_lock(stu, (Gc *) name);
    return Type_new(stu, name, arity);
}

extern long
Type_name(Stu *stu, Type t)
{
    return stu->type_registry.name[t.i];
}

extern unsigned
Type_arity(Stu *stu, Type t)
{
    return stu->type_registry.arity[t.i];
}

extern Sv
*Type_value(Stu *stu, Type t)
{
    return stu->type_registry.value[t.i];
}
