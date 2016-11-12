#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <stdio.h>

#include "sv.h"

static void Sv_cons_dump(Sv *);

extern Sv
*Sv_new(enum Sv_type type)
{
    Sv *x = NULL;
    int i;

    if ((x = malloc(sizeof(*x))) != NULL) {
        x->gc = 0;
        x->type = type;

        for (i = 0; i < SV_SEXP_REGISTERS; i++)
            x->val.reg[i] = NULL;
    } else {
        err(1, "Sv_new");
    }

    return x;
}

extern Sv
*Sv_new_int(int i)
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

extern void
Sv_destroy(Sv **sv)
{
    int i;

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

        case SV_SEXP:
            for (i = 0; i < SV_SEXP_REGISTERS; i++) {
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

        case SV_SEXP:
            printf("(");
            Sv_cons_dump(sv);
            printf(")");
            break;

        case SV_INT:
            printf("%d", sv->val.i);
            break;

        case SV_FUNC:
            break;
        }
    } else {
        printf("nil");
    }
}

static void
Sv_cons_dump(Sv *sv)
{
    Sv *car = Sv_car(sv);
    Sv *cdr = Sv_cdr(sv);
    Sv_dump(car);
    if (cdr) {
        if (cdr->type == SV_SEXP) {
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
    Sv *z = Sv_new(SV_SEXP);
    if (z != NULL) {
        z->val.reg[SV_CAR] = x;
        z->val.reg[SV_CDR] = y;
    } else {
        err(1, "Sv_cons");
    }

    return z;
}

extern Sv *Sv_car(Sv *x)
{
    if (!x || x->type != SV_SEXP) {
        errx(1, "car: sexp argument required");
    }

    return x->val.reg[SV_CAR];
}

extern Sv *Sv_cdr(Sv *x)
{
    if (!x || x->type != SV_SEXP) {
        errx(1, "cdr: sexp argument required");
    }

    return x->val.reg[SV_CDR];
}
