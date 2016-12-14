#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "builtins.h"

struct Builtin {
    const char *name;
    Sv_func func;
};

extern void
Builtin_init(void)
{
    struct Builtin *b = NULL;
    struct Builtin builtins[] = {
        { "+",       Builtin_add },
        { "-",       Builtin_sub },
        { "*",       Builtin_mul },
        { "/",       Builtin_div },
        { "quote",   Builtin_quote },
        { "def",     Builtin_def },
        { "cons",    Builtin_cons },
        { "list",    Builtin_list },
        { u8"λ",     Builtin_lambda },
        { "lambda",  Builtin_lambda },
        { "eval",    Builtin_eval },
        { "car",     Builtin_car },
        { "cdr",     Builtin_cdr },
        { "reverse", Builtin_reverse },
        { "if",      Builtin_if },
        { "=",       Builtin_eq },
        { ">",       Builtin_gt },
        { "<",       Builtin_lt },
        { ">=",      Builtin_gte },
        { "<=",      Builtin_lte },
        { NULL }
    };

    for (b = builtins; b->name != NULL; b++) {
        Env_main_put(Sv_new_sym(b->name), Sv_new_func(b->func));
    }
}

extern Sv
*Builtin_add(Env *env, Sv *x)
{
    Sv *cur = NULL;
    long acc = 0;

    while (!IS_NIL(x) && (cur = CAR(x))) {
        if (cur->type != SV_INT)
            return Sv_new_err("'+' can operate on numbers only");
        acc += cur->val.i;
        x = CDR(x);
    }

    return Sv_new_int(acc);
}

extern Sv
*Builtin_sub(Env *env, Sv *x)
{
    Sv *cur = NULL;
    long acc = 0;

    if (!IS_NIL(x) && (cur = CAR(x))) {
        if (cur->type != SV_INT)
            return Sv_new_err("'-' can operate on numbers only");
        acc = cur->val.i;
        x = CDR(x);

        if (x) {
            while (!IS_NIL(x) && (cur = CAR(x))) {
                if (cur->type != SV_INT)
                    return Sv_new_err("'-' can operate on numbers only");
                acc -= cur->val.i;
                x = CDR(x);
            }
        } else {
            acc = -acc;
        }
    }

    return Sv_new_int(acc);
}

extern Sv
*Builtin_mul(Env *env, Sv *x)
{
    Sv *cur = NULL;
    long acc = 1;

    while (!IS_NIL(x) && (cur = CAR(x))) {
        if (cur->type != SV_INT)
            return Sv_new_err("'*' can operate on numbers only");
        acc *= cur->val.i;
        x = CDR(x);
    }

    return Sv_new_int(acc);
}

extern Sv
*Builtin_div(Env *env, Sv *x)
{
    Sv *cur = NULL;
    long acc = 0;

    if (!IS_NIL(x) && (cur = CAR(x))) {
        if (cur->type != SV_INT)
            return Sv_new_err("'/' can operate on numbers only");
        acc = cur->val.i;
        x = CDR(x);

        if (x) {
            while (!IS_NIL(x) && (cur = CAR(x))) {
                if (cur->type != SV_INT)
                    return Sv_new_err("'/' can operate on numbers only");
                if (cur->val.i == 0)
                    return Sv_new_err("'/' cannot divide by zero!");
                acc /= cur->val.i;
                x = CDR(x);
            }
        } else {
            acc = 1 / acc;
        }
    } else {
        return Sv_new_err("'/' requires one or more arguments");
    }

    return Sv_new_int(acc);
}

extern Sv
*Builtin_quote(Env *env, Sv *x)
{
    return CAR(x);
}

extern Sv
*Builtin_def(Env *env, Sv *x)
{
    Sv *y = CAR(x);
    Sv *z = CDR(x);

    if (y && y->type != SV_SYM)
        y = Sv_eval(env, y);

    /* We need a symbol in the head. */
    if (!y || y->type != SV_SYM)
        return Sv_new_err("'def' needs a symbol as the first argument");

    /* Def in the top scope. */
    z = Sv_eval(env, CAR(z));
    Env_main_put(y, z);

    /*
     * If we installed a lambda, also install in the lambda's env so
     * it can call itself.
     */
    if (z->type == SV_LAMBDA)
        z->val.ufunc->env = Env_put(z->val.ufunc->env, y, z);

    return NIL;
}

extern Sv
*Builtin_cons(Env *env, Sv *x)
{
    Sv *y = CAR(x), *z = CADR(x);

    if (!x || !y)
        return Sv_new_err("'cons' needs two arguments");

    return Sv_cons(y, z);
}

extern Sv
*Builtin_list(Env *env, Sv *x)
{
    return Sv_list(x);
}

extern Sv
*Builtin_lambda(Env *env, Sv *x)
{
    Sv *formals = NULL, *cur = NULL;

    if (!x)
        return x;

    /* All formals should be symbols. */
    formals = CAR(x);
    if (formals->type == SV_CONS) {
        while (!IS_NIL(formals) && formals->type == SV_CONS && (cur = CAR(formals))) {
            if (cur->type != SV_SYM) {
                return Sv_new_err(
                    "'lambda' formals need to be symbols");
            }
            formals = CDR(formals);
        }

    } else {
        return Sv_new_err(
            "'lambda' needs a list of symbols as the first argument");
    }

    formals = CAR(x);
    cur = CDR(x);

    return Sv_new_lambda(env, formals, CAR(cur));
}

extern Sv
*Builtin_eval(Env *env, Sv *x)
{
    return x ? Sv_eval(env, CAR(x)) : x;
}

extern Sv
*Builtin_car(Env *env, Sv *x)
{
    if (!(x = CAR(x)) || x->type != SV_CONS)
        return Sv_new_err("'car' needs a single list argument");
    return CAR(x);
}

extern Sv
*Builtin_cdr(Env *env, Sv *x)
{
    if (!(x = CAR(x)) || x->type != SV_CONS)
        return Sv_new_err("'cdr' needs a single list argument");
    return CDR(x);
}

extern Sv
*Builtin_reverse(Env *env, Sv *x)
{
    if (!(x = CAR(x)) || x->type != SV_CONS)
        return Sv_new_err("'reverse' needs a single list argument");
    return Sv_reverse(x);
}

extern Sv
*Builtin_if(Env *env, Sv* sv)
{
    Sv *cond = CAR(sv), *first = CADR(sv), *second = CADDR(sv);

    if (!cond || !first)
        return NULL;

    cond = Sv_eval(env, cond);
    if (!cond || cond->type != SV_BOOL)
        return Sv_new_err("'if' condition must evaluate to a bool");

    if (cond->val.i) {
        return Sv_eval(env, first);
    } else {
        return second ? Sv_eval(env, second) : NULL;
    }
}

/*
 * Generate comparison functions using a macro.
 */

#define OP_EQ  1
#define OP_GT  2
#define OP_LT  3
#define OP_GTE 4
#define OP_LTE 5

#define CMP_SV(op) \
    Sv *x = CAR(sv), *y = CADR(sv); \
    if (!x && !y) return Sv_new_bool(1); \
    if (x && y && x->type == y->type) { \
        switch (x->type) { \
        case SV_NIL: \
            return Sv_new_bool(1); \
        case SV_INT: \
        case SV_BOOL: \
        case SV_SYM: \
            return Sv_new_bool(compare_numbers(op, x->val.i, y->val.i)); \
        case SV_STR: \
        case SV_ERR: \
            return Sv_new_bool(compare_strings(op, x->val.buf, y->val.buf)); \
        default: \
            return Sv_new_err("'eq' does not support these types"); \
        } \
    } \
    return Sv_new_bool(0);

static int
compare_numbers(int op, int x, int y)
{
    switch (op) {
    case OP_EQ:
        return x == y ? 1 : 0;

    case OP_GT:
        return x > y ? 1 : 0;

    case OP_LT:
        return x < y ? 1 : 0;

    case OP_GTE:
        return x >= y ? 1 : 0;

    case OP_LTE:
        return x <= y ? 1 : 0;

    default:
        return 0;
    }
}

static int
compare_strings(int op, const char *x, const char *y)
{
    switch (op) {
    case OP_EQ:
        return !strcmp(x, y) ? 1 : 0;

    case OP_GT:
        return strcmp(x, y) > 0 ? 1 : 0;

    case OP_LT:
        return strcmp(x, y) < 0 ? 1 : 0;

    case OP_GTE:
        return strcmp(x, y) >= 0 ? 1 : 0;

    case OP_LTE:
        return strcmp(x, y) <= 0 ? 1 : 0;

    default:
        return 0;
    }
}

extern Sv
*Builtin_eq(Env *env, Sv* sv)
{
    CMP_SV(OP_EQ);
}

extern Sv
*Builtin_gt(Env *env, Sv* sv)
{
    CMP_SV(OP_GT);
}

extern Sv
*Builtin_lt(Env *env, Sv* sv)
{
    CMP_SV(OP_LT);
}

extern Sv
*Builtin_gte(Env *env, Sv* sv)
{
    CMP_SV(OP_GTE);
}

extern Sv
*Builtin_lte(Env *env, Sv* sv)
{
    CMP_SV(OP_LTE);
}
