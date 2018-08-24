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
#include "types.h"

static Sv
*quote(Stu *stu, Env *env, Sv *args)
{
    if (args->type != SV_CONS || CDR(args) != NIL)
        return Sv_new_err(stu, "quote requires a single argument");
    return CAR(args);
}

static int
is_def_pattern(Sv *sv) {
    switch (sv->type) {
    case SV_SYM:
         return 1;
    case SV_VECTOR:
        for (long i = 0; i < sv->val.vector->length; ++i)
            if (!is_def_pattern(sv->val.vector->values[i]))
                return 0;
        return 1;
    case SV_NIL:
        return 1;
    case SV_CONS:
        return is_def_pattern(CAR(sv)) ? is_def_pattern(CDR(sv)) : 0;
    default:
        return 0;
    }
}

static Sv
*bind_def_pattern(Stu *stu, Sv *lhs, Sv *rhs) {
    Sv *res = NULL;
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
    case SV_NIL:
        if (rhs->type == SV_NIL)
            return NIL;
        return Sv_new_err(stu, "'def' expected nil but got something else");
    case SV_CONS:
        if (rhs->type != SV_CONS)
            return Sv_new_err(stu, "'def' expected a cons cell as a result");
        res = bind_def_pattern(stu, CAR(lhs), CAR(rhs));
        if (res->type != SV_NIL)
            /* Return early in case something went wrong */
            return res;
        return bind_def_pattern(stu, CDR(lhs), CDR(rhs));
    case SV_VECTOR:
        if (rhs->type != SV_VECTOR)
            return Sv_new_err(stu, "'def' expected a vector as a result");
        if (lhs->val.vector->length != rhs->val.vector->length)
            return Sv_new_err(stu, "'def' mismatch in vector lengths");
        for (long i = 0; i < lhs->val.vector->length; ++i) {
            res = bind_def_pattern(stu, lhs->val.vector->values[i], rhs->val.vector->values[i]);
            if (res->type != SV_NIL)
                /* Return early in case something went wrong */
                return res;
        }
        return NIL;
    default:
        return Sv_new_err(stu, "'def' needs a pattern as the first argument");
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

    /* We need a pattern in the head. */
    if (!y || !is_def_pattern(y)) {
        return Sv_new_err(stu, "'def' needs a pattern as the first argument");
    }

    z = Sv_eval(stu, env, z);

    /* Record the new binding(s) and return NIL. */
    return bind_def_pattern(stu, y, z);
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
*defun(Stu *stu, Env *env, Sv *args)
{
    if (args->type != SV_CONS)
        return Sv_new_err(stu, "'defun' args is not a cons");

    Sv *name = CAR(args);
    Sv *formals = CADR(args);
    Sv *body = CADDR(args);

    Sv *lambda_def = lambda(stu, env, Sv_cons(stu, formals, Sv_cons(stu, body, NIL)));
    Sv *def_args = Sv_cons(stu, name, Sv_cons(stu, lambda_def, NIL));

    return def(stu, env, def_args);
}

static Sv
*deftype(Stu *stu, Env *env, Sv *args)
{
    if (args->type != SV_CONS)
        return Sv_new_err(stu, "deftype args is not a cons");

    Sv *type_name = CAR(args);

    if (type_name->type != SV_SYM)
        return Sv_new_err(stu, "deftype first argument is not a symbol");

    Sv *field_vector = Sv_new_vector(stu, CDR(args));

    if (field_vector->type == SV_ERR)
        /* Return error in case something went wrong */
        return field_vector;

    Sv_type type = Type_new(stu, type_name, field_vector);
    Sv *constructor = Sv_new_structure_constructor(stu, type);
    Env_capture(stu, type_name, constructor);

    return NIL;
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
    Env_main_put(stu, name, lamb);

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
*try(Stu *stu, Env *env, Sv *x)
{
    return Try_eval_stu_catch(stu, env, x);
}

static Sv
*open(Stu *stu, Env *env, Sv *args) {
    if (args->type != SV_CONS)
        return Sv_new_err(stu, "'open' args is not a cons");
    if (!IS_NIL(CDR(args)))
        return Sv_new_err(stu, "'open' should only have one argument");
    Sv *x = Sv_eval(stu, env, CAR(args));
    if (x->type < SV_BUILTIN_TYPE_END)
        return Sv_new_err(stu, "Value to be opened must be a structure");
    Sv_vector *fields = Type_field_vector(stu, x->type)->val.vector;
    Sv **values = x->val.structure;
    for (long i = 0; i < fields->length; ++i)
        Env_capture(stu, fields->values[i], values[i]);
    return NIL;
}

static char *sym_strings[] = {
    "nil",  // Not a special form, but already taken as symbol 0
    "quote",
    "def",
    "deftype",
    "defmacro",
    "defun",
    "lambda",
    "λ",
    "if",
    "try",
    "open",
    ""
};

static Special_form_f funcs[] = {
    NULL,
    quote,
    def,
    deftype,
    defmacro,
    defun,
    lambda,
    lambda,
    stu_if,
    try,
    open
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
