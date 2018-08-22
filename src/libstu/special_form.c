/*
 * Copyright (c) 2016, 2017 Mikey Austin <mikey@jackiemclean.net>
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

#include <err.h>

#include "env.h"
#include "special_form.h"
#include "stu_private.h"
#include "try.h"
#include "sv.h"
#include "gc.h"
#include "symtab.h"

static Sv
*quote(Stu *stu, Env *env, Sv *args)
{
    if (args->type != SV_CONS || CDR(args) != NIL)
        return Sv_new_err(stu, "quote requires a single argument");
    return CAR(args);
}

static int
is_symbol_vector_tree(Sv *sv) {
    switch (sv->type) {
    case SV_SYM:
         return 1;
    case SV_VECTOR:
        for (long i = 0; i < sv->val.vector->length; ++i)
            if (!is_symbol_vector_tree(sv->val.vector->values[i]))
                return 0;
        return 1;
    default:
        return 0;
    }
}

static Sv
*bind_def_vector_tree(Stu *stu, Sv *lhs, Sv *rhs) {
    switch (lhs->type) {
    case SV_SYM:
        Env_capture(stu, lhs, rhs);
        /*
         * If we installed a lambda, also install in the lambda's env so
         * it can call itself.
         */
        if (rhs->type == SV_LAMBDA)
            rhs->val.ufunc->env = Env_put(stu, rhs->val.ufunc->env, lhs, rhs);
        return NIL;
    case SV_VECTOR:
        if (rhs->type != SV_VECTOR)
            return Sv_new_err(stu, "'def' expected a vector as a result");
        if (lhs->val.vector->length != rhs->val.vector->length)
            return Sv_new_err(stu, "'def' mismatch in vector lengths");
        for (long i = 0; i < lhs->val.vector->length; ++i)
            bind_def_vector_tree(stu, lhs->val.vector->values[i], rhs->val.vector->values[i]);
        return NIL;
    default:
        return Sv_new_err(stu, "'def' needs a symbol or a vector of symbols as the first argument");
    }
}

static Sv
*def(Stu *stu, Env *env, Sv *args)
{
    Sv *y = NULL, *z = NULL;
    if (args->type != SV_CONS)
        return Sv_new_err(stu, "'def' args is not a cons");

    y = CAR(args);
    z = CADR(args);

    /* We need a symbol or a vector of symbols in the head. */
    if (!y || !is_symbol_vector_tree(y)) {
        return Sv_new_err(stu, "'def' needs a symbol or a vector of symbols as the first argument");
    }

    z = Sv_eval(stu, env, z);

    /* Record the new binding(s) and return NIL. */
    return bind_def_vector_tree(stu, y, z);
}

static Sv
*lambda(Stu *stu, Env *env, Sv *args)
{
    Sv *formals = NULL, *cur = NULL;
    if (args->type != SV_CONS)
        return Sv_new_err(stu, "'lambda' args is not a cons");

    /* All formals should be symbols. */
    formals = CAR(args);
    if (formals->type == SV_CONS || IS_NIL(formals)) {
        while (!IS_NIL(formals) && formals->type == SV_CONS && (cur = CAR(formals))) {
            if (cur->type != SV_SYM) {
                return Sv_new_err(
                    stu, "'lambda' formals need to be symbols");
            }
            formals = CDR(formals);
        }
    } else {
        return Sv_new_err(
            stu, "'lambda' needs a list of symbols as the first argument");
    }

    formals = CAR(args);
    cur = CDR(args) ;

    return Sv_new_lambda(stu, env, formals, cur);
}

static Sv
*defmacro(Stu *stu, Env *env, Sv *args)
{
    if (args->type != SV_CONS)
        return Sv_new_err(stu, "'defmacro' args is not a cons");

    Sv *name = CAR(args);
    if (!name || name->type != SV_SYM)
        return Sv_new_err(stu, "'defmacro' needs a symbol as the first argument");

    Sv *lamb = lambda(stu, env, CDR(args));

    lamb->val.ufunc->is_macro = 1;
    Env_capture(stu, name, lamb);

    return NIL;
}

static Sv
*stu_if(Stu *stu, Env *env, Sv *args)
{
    if (args->type != SV_CONS)
        return Sv_new_err(stu, "'if' args is not a cons");

    Sv *cond = CAR(args);
    Sv *clauses = CDR(args);

    if (clauses->type != SV_CONS)
        return Sv_new_err(stu, "'if' clauses is not a cons");

    Sv *first = CAR(clauses), *second = CDR(clauses);

    cond = Sv_eval(stu, env, cond);
    if (!cond || cond->type != SV_BOOL)
        return Sv_new_err(stu, "'if' condition must evaluate to a bool");

    if (cond->val.i) {
        return Sv_eval(stu, env, first);
    } else {
        return IS_NIL(second) ? second : Sv_eval(stu, env, CAR(second));
    }
}

static Sv
*progn(Stu *stu, Env *env, Sv *x)
{
    return Sv_eval_list(stu, env, x);
}

static Sv
*try(Stu *stu, Env *env, Sv *x)
{
    return Try_eval_stu_catch(stu, env, x);
}

static char *sym_strings[] = {
    "nil",  // Not a special form, but already taken as symbol 0
    "quote",
    "def",
    "defmacro",
    "lambda",
    "λ",
    "if",
    "progn",
    "try"
};

static Special_form_f funcs[] = {
    NULL,
    quote,
    def,
    defmacro,
    lambda,
    lambda,
    stu_if,
    progn,
    try
};

#define SYM_STRINGS_SIZE (sizeof(sym_strings) / sizeof(*sym_strings))

extern void
Special_form_register_symbols(Stu *stu)
{
    for (long i = 1; i < SYM_STRINGS_SIZE; ++i)
        if (Symtab_get_id(stu, sym_strings[i]) != i)
            err(1, "Special_form_register_symbols");
}

extern Special_form_f
Special_form_get_f(Stu *stu, Sv *sv)
{
    long i = sv->val.i;
    if (i < SYM_STRINGS_SIZE)
        return funcs[i];
    return NULL;
}
