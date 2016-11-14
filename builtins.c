#include <stdlib.h>

#include "builtins.h"

struct Builtin {
    const char *name;
    Sv_func func;
};

extern void
Builtin_install(Env *env)
{
    struct Builtin *b = NULL;
    struct Builtin builtins[] = {
        { "+",       Builtin_add },
        { "-",       Builtin_sub },
        { "*",       Builtin_mul },
        { "/",       Builtin_div },
        { "quote",   Builtin_quote },
        { "cons",    Builtin_cons },
        { "list",    Builtin_list },
        { "car",     Builtin_car },
        { "cdr",     Builtin_cdr },
        { "reverse", Builtin_reverse },
        NULL
    };

    for (b = builtins; b->name != NULL; b++) {
        Env_put(env, Sv_new_sym(b->name), Sv_new_func(b->func));
    }
}

extern Sv
*Builtin_add(Env *env, Sv *x)
{
    Sv *cur = NULL;
    int acc = 0;

    while (x && (cur = CAR(x))) {
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
    int acc;

    if (x && (cur = CAR(x))) {
        if (cur->type != SV_INT)
            return Sv_new_err("'-' can operate on numbers only");
        acc = cur->val.i;
        x = CDR(x);

        while (x && (cur = CAR(x))) {
            if (cur->type != SV_INT)
                return Sv_new_err("'-' can operate on numbers only");
            acc -= cur->val.i;
            x = CDR(x);
        }
    }

    return Sv_new_int(acc);
}

extern Sv
*Builtin_mul(Env *env, Sv *x)
{
    Sv *cur = NULL;
    int acc = 1;

    while (x && (cur = CAR(x))) {
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
    int acc;

    if (x && (cur = CAR(x))) {
        if (cur->type != SV_INT)
            return Sv_new_err("'/' can operate on numbers only");
        acc = cur->val.i;
        x = CDR(x);

        while (x && (cur = CAR(x))) {
            if (cur->type != SV_INT)
                return Sv_new_err("'/' can operate on numbers only");
            if (cur->val.i == 0)
                return Sv_new_err("'/' cannot divide by zero!");
            acc /= cur->val.i;
            x = CDR(x);
        }
    }

    return Sv_new_int(acc);
}

extern Sv
*Builtin_quote(Env *env, Sv *x)
{
    return x;
}

extern Sv
*Builtin_cons(Env *env, Sv *x)
{
    Sv *y = CAR(x), *z = CDR(x);

    if (!y || !z || !(z = CAR(z)))
        return Sv_new_err("'cons' needs two arguments");

    return Sv_cons(y, z);
}

extern Sv
*Builtin_list(Env *env, Sv *x)
{
    Sv *y = NULL, *z = NULL;

    while (x && (y = CAR(x))) {
        z = Sv_cons(y, z);
        x = CDR(x);
    }

    return Sv_reverse(z);
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
