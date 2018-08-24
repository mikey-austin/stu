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
#include "symtab.h"
#include "types.h"
#include "utils.h"

#include <stdlib.h>

#define INITIAL_CAPACITY 20
#define GROWTH_FACTOR 2

extern void
Type_registry_init(Type_registry *reg)
{
   reg->name_symbol = CHECKED_CALLOC(INITIAL_CAPACITY, sizeof(*(reg->name_symbol)));
   reg->field_vectors = CHECKED_CALLOC(INITIAL_CAPACITY, sizeof(*(reg->field_vectors)));
   reg->size = 0;
   reg->capacity = INITIAL_CAPACITY;
}

extern void
Type_registry_release(Type_registry *reg)
{
    free(reg->name_symbol);
    free(reg->field_vectors);
    reg->name_symbol = NULL;
    reg->field_vectors = NULL;
}

static void
resize_arrays(Type_registry *reg)
{
    reg->capacity *= GROWTH_FACTOR;
    reg->name_symbol = CHECKED_REALLOC(reg->name_symbol, reg->capacity * sizeof(*(reg->name_symbol)));
    reg->field_vectors = CHECKED_REALLOC(reg->field_vectors, reg->capacity * sizeof(*(reg->field_vectors)));
}

extern Sv_type
Type_new(Stu *stu, Sv *name, Sv *fields)
{
    Type_registry *reg = &(stu->type_registry);

    if (reg->size == reg->capacity)
        resize_arrays(reg);

    Gc_lock(stu, (Gc *) name);
    Gc_lock(stu, (Gc *) fields);
    for (long i = 0; i < fields->val.vector->length; ++i)
        Gc_lock(stu, (Gc *) fields->val.vector->values[i]);

    long index = reg->size++;
    reg->name_symbol[index] = name;
    reg->field_vectors[index] = fields;

    return SV_BUILTIN_TYPE_END + index;
}

extern Sv
*Type_name_symbol(Stu *stu, Sv_type t)
{
    return stu->type_registry.name_symbol[t - SV_BUILTIN_TYPE_END];
}

extern const char
*Type_name_string(Stu *stu, Sv_type t)
{
    return Symtab_get_name(stu, Type_name_symbol(stu, t)->val.i);
}

extern Sv
*Type_field_vector(Stu *stu, Sv_type t)
{
    return stu->type_registry.field_vectors[t - SV_BUILTIN_TYPE_END];
}

extern long
Type_field_index(Stu *stu, Sv_type t, Sv *field)
{
    Sv_vector *vec = Type_field_vector(stu, t)->val.vector;
    long sym_val = field->val.i;
    for (long i = 0; i < vec->length; ++i)
        if (sym_val == vec->values[i]->val.i)
            return i;
    return vec->length;
}
