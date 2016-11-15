#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <stdio.h>

#include "sv.h"
#include "env.h"

static void Sv_cons_dump(Sv *);

extern Sv
*Sv_new(enum Sv_type type)
{
    Sv *x = NULL;
    int i;

    if ((x = malloc(sizeof(*x))) != NULL) {
        x->type = type;
        for (i = 0; i < SV_CONS_REGISTERS; i++)
            x->val.reg[i] = NULL;
    } else {
        err(1, "Sv_new");
    }

    return x;
}

extern Sv
*Sv_new_int(long i)
{
    Sv *x = Sv_new(SV_INT);
    x->val.i = i;
    return  x;
}

extern Sv
*Sv_new_str(const char *str)
{
    Sv *x = Sv_new(SV_STR);
    if ((x->val.buf = strdup(str)) == NULL)
        err(1, "Sv_new_str");
    return  x;
}

extern Sv
*Sv_new_sym(const char *sym)
{
    Sv *x = Sv_new_str(sym);
    x->type = SV_SYM;
    return x;
}

extern Sv
*Sv_new_err(const char *err)
{
    Sv *x = Sv_new_str(err);
    x->type = SV_ERR;
    return x;
}

extern Sv
*Sv_new_func(Sv_func func)
{
    Sv *x = Sv_new(SV_FUNC);
    x->val.func = func;
    return x;
}

extern void
Sv_destroy(Sv **sv)
{
    if (sv && *sv) {
        switch ((*sv)->type) {
        case SV_ERR:
        case SV_SYM:
        case SV_STR:
            if ((*sv)->val.buf) {
                free((*sv)->val.buf);
                (*sv)->val.buf = NULL;
            }
            break;

        case SV_SPECIAL:
        case SV_CONS:
            for (int i = 0; i < SV_CONS_REGISTERS; i++) {
                Sv_destroy(&((*sv)->val.reg[i]));
                (*sv)->val.reg[i] = NULL;
            }
            break;

        case SV_INT:
        case SV_FUNC:
            break;
        }

        free(*sv);
        *sv = NULL;
    }
}

extern void
Sv_dump(Sv *sv)
{
    if (sv) {
        switch (sv->type) {
        case SV_SYM:
        case SV_ERR:
        case SV_STR:
            if (sv->val.buf)
                printf("%s", sv->val.buf);
            break;

        case SV_SPECIAL:
        case SV_CONS:
            printf("(");
            Sv_cons_dump(sv);
            printf(")");
            break;

        case SV_INT:
            printf("%ld", sv->val.i);
            break;

        case SV_FUNC:
            printf("<function>");
            break;
        }
    } else {
        printf("nil");
    }
}

static void
Sv_cons_dump(Sv *sv)
{
    Sv *car = CAR(sv);
    Sv *cdr = CDR(sv);
    Sv_dump(car);
    if (cdr) {
        if (cdr->type == SV_CONS) {
            printf(" ");
            Sv_cons_dump(cdr);
        } else {
            printf(" . ");
            Sv_dump(cdr);
        }
    }
}

extern Sv
*Sv_cons(Sv *x, Sv *y)
{
    Sv *z = Sv_new(SV_CONS);
    if (z != NULL) {
        z->val.reg[SV_CAR_REG] = x;
        z->val.reg[SV_CDR_REG] = y;
    } else {
        err(1, "Sv_cons");
    }

    return z;
}

extern Sv
*Sv_special(Sv *x, Sv *y)
{
    Sv *z = Sv_cons(x, y);
    z->type = SV_SPECIAL;
    return z;
}

extern Sv
*Sv_reverse(Sv *x)
{
    Sv *y = NULL;

    while (x && x->type == SV_CONS) {
        y = Sv_cons(CAR(x), y);
        x = CDR(x);
    }

    return y;
}

extern Sv
*Sv_eval(Env *env, Sv *x)
{
    Sv *y = NULL;

    if (!x)
        return x;

    switch (x->type) {
    case SV_SYM:
        if ((y = Env_get(env, x)) == NULL) {
            y = Sv_new_err("possibly unknown symbol");
        }
        return y;

    case SV_SPECIAL:
    case SV_CONS:
        return Sv_eval_sexp(env, x);

    default:
        return x;
    }
}

extern Sv
*Sv_eval_sexp(Env *env, Sv *x)
{
    Sv *cur = x, *y = NULL;

    if (!cur)
        return NULL;

    switch (cur->type) {
    case SV_CONS:
        /* Evaluate all arguments. */
        do {
            if (cur->type == SV_CONS) {
                y = Sv_cons(Sv_eval(env, CAR(cur)), y);
                cur = CDR(cur);
            } else {
                y = Sv_cons(cur, y);
                break;
            }
        } while (cur);
        x = Sv_reverse(y);
        y = CAR(x);
        break;

    case SV_SPECIAL:
        /* Only evaluate the head, leaving tail intact. */
        y = Sv_eval(env, CAR(x));
        break;

    default:
        break;
    }

    /* The car should now be a function. */
    if (y->type != SV_FUNC)
        return Sv_new_err("first element is not a function");

    return y->val.func(env, CDR(x));
}
