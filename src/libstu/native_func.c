/*
 * Copyright (c) 2016, 2017 Mikey Austin <mikey@jackiemclean.net>
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

#include "native_func.h"
#include "stu_private.h"
#include "env.h"

typedef struct Sv_native_func {
    Sv_native_func_t func;
    unsigned arity, rest;
} Sv_native_func;

typedef struct Sv_native_closure {
    Sv_native_func_t func;
    unsigned arity, rest, bound_num;
    Sv *bound_args[];
} Sv_native_closure;

static void
resize_args(Stu *stu, unsigned size)
{
    Sv **array = realloc(stu->native_func_args, size * sizeof(Sv*));
    if (array == NULL)
        return;
    stu->native_func_args = array;
    stu->native_func_args_capacity = size;
}

extern Sv_native_func
*Sv_native_func_new(Stu *stu, Sv_native_func_t func, unsigned arity, unsigned rest)
{
    Sv_native_func *f = malloc(sizeof *f);
    if (f == NULL)
        return NULL;
    f->func = func;
    f->arity = arity;
    f->rest = rest;
    if (arity > stu->native_func_args_capacity)
        resize_args(stu, arity);
    return f;
}

static Sv
*Native_call(Stu *stu, Env *env, Sv_native_func_t f,
             unsigned arity, unsigned rest, unsigned bound_num,
             Sv **arg_array, Sv *args)
{
    if (rest)
        --arity;
    while (!IS_NIL(args) && arity > 0) {
        *(arg_array++) = CAR(args);
        args = CDR(args);
        --arity;
        ++bound_num;
    }

    if (arity > 0) {
        Sv_native_closure *c = malloc(sizeof(*c) + bound_num * sizeof(Sv*));
        if (c != NULL) {
            c->arity = arity;
            c->rest = rest;
            c->func = f;
            c->bound_num = bound_num;
            memcpy(c->bound_args, stu->native_func_args, bound_num);
            Sv *sv = Sv_new(stu, SV_NATIVE_CLOS);
            sv->val.clos = c;
            return sv;
        }
        return NULL;
    } else {
        if (rest) {
            *arg_array = args;
            ++bound_num;
        }
        else if (!IS_NIL(args))
            return NULL;
        return f(stu, env, stu->native_func_args);
    }
}

extern Sv
*Sv_native_func_call(Stu *stu, Env *env, Sv_native_func *f, Sv *args)
{
    return Native_call(
        stu, env, f->func, f->arity, f->rest, 0, stu->native_func_args, args);
}

extern Sv
*Sv_native_closure_call(Stu *stu, Env *env, Sv_native_closure *f, Sv *args)
{
    memcpy(stu->native_func_args, f->bound_args, f->bound_num);
    return Native_call(
        stu, env, f->func, f->arity, f->rest, f->bound_num,
        stu->native_func_args + f->bound_num, args);
}

extern Sv
*Sv_native_func_register(Stu *stu, const char *name, Sv_native_func_t f, unsigned args, unsigned rest)
{
    Sv *x = Sv_new_native_func(stu, f, args, rest);
    Env_main_put(stu, Sv_new_sym(stu, name), x);
    return x;
}
