/*
 * Copyright (c) 2016 - 2018 Mikey Austin <mikey@jackiemclean.net>
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

#include <stdlib.h>
#include <err.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "alloc/alloc.h"
#include "env.h"
#include "gc.h"
#include "native_func.h"
#include "special_form.h"
#include "stu_private.h"
#include "sv.h"
#include "symtab.h"
#include "utils.h"
#include "call_stack.h"

extern Sv
*Sv_new(Stu *stu, Sv_type type)
{
    Sv *x = NULL;

    if ((x = Alloc_allocate(stu->sv_alloc)) != NULL) {
        GC_INIT(stu, x, GC_TYPE_SV);
        x->type = type;
    } else {
        err(1, "Sv_new");
    }

    return x;
}

extern Sv
*Sv_new_int(Stu *stu, long i)
{
    Sv *x = Sv_new(stu, SV_INT);
    x->val.i = i;
    return x;
}

extern Sv
*Sv_new_float(Stu *stu, double f)
{
    Sv *x = Sv_new(stu, SV_FLOAT);
    x->val.f = f;
    return x;
}

extern Sv
*Sv_new_rational(Stu *stu, long n, long d)
{
    if (n % d == 0)
        return Sv_new_int(stu, n / d);

    Sv *x = Sv_new(stu, SV_RATIONAL);
    long max_search = labs(n) > d ? d : labs(n);
    long cur = 2;

    while (cur <= max_search) {
        if ((n % cur == 0) && (d % cur == 0)) {
            n /= cur;
            d /= cur;
        } else {
            cur++;
        }
    }

    x->val.rational.n = n;
    x->val.rational.d = d;

    return x;
}

extern Sv
*Sv_new_bool(Stu *stu, short i)
{
    Sv *x = Sv_new(stu, SV_BOOL);
    x->val.i = (int) i;
    return x;
}

extern Sv
*Sv_new_str(Stu *stu, const char *str)
{
    Sv *x = Sv_new(stu, SV_STR);
    if ((x->val.buf = strdup(str)) == NULL)
        err(1, "Sv_new_str");
    return x;
}

extern Sv
*Sv_new_sym(Stu *stu, const char *sym)
{
    Sv *x = Sv_new(stu, SV_SYM);
    x->val.i = Symtab_get_id(stu, sym);
    return x;
}

extern Sv
*Sv_new_sym_from_id(Stu *stu, long id)
{
    Sv *x = Sv_new(stu, SV_SYM);
    x->val.i = id;
    return x;
}

extern Sv
*Sv_new_err(Stu *stu, const char *err)
{
    Sv *x = Sv_new_str(stu, err);
    x->type = SV_ERR;
    return x;
}

extern Sv
*Sv_new_native_func(Stu *stu, Sv_native_func_t f, unsigned arity, unsigned rest)
{
    Sv *x = Sv_new(stu, SV_NATIVE_FUNC);
    x->val.func = Sv_native_func_new(stu, f, arity, rest);
    return x;
}

extern Sv
*Sv_new_special(Stu *stu, enum Sv_special_type type, Sv *body)
{
    Sv *x = Sv_new(stu, SV_SPECIAL);
    Sv_special *s = Alloc_allocate(stu->sv_special_alloc);

    s->type = type;
    s->body = Sv_copy(stu, body);
    x->val.special = s;

    return x;
}

extern Sv
*Sv_new_lambda(Stu *stu, Env *env, Sv *formals, Sv *body)
{
    Sv *x = Sv_new(stu, SV_LAMBDA);
    Sv_ufunc *f = Alloc_allocate(stu->sv_ufunc_alloc);

    f->env = env;
    f->formals = formals;
    f->body = body;
    f->is_macro = 0;
    x->val.ufunc = f;

    return x;
}

extern Sv
*Sv_new_vector(struct Stu *stu, Sv *sv)
{
    long count = 0;
    for (Sv *tmp = sv; !IS_NIL(tmp); tmp = CDR(tmp))
        if (tmp->type == SV_CONS) {
            if (count == LONG_MAX)
                return Sv_new_err(stu, "Vector exceeds maximum allowed size");
            ++count;
        } else {
            return Sv_new_err(stu, "Vector argument is not a proper list");
        }

    Sv *x = Sv_new(stu, SV_VECTOR);
    Sv_vector *vec = CHECKED_MALLOC(sizeof(*vec) + count * sizeof(Sv*));

    x->val.vector = vec;
    vec->length = count;
    long i = 0;
    for (Sv *tmp = sv; !IS_NIL(tmp); tmp = CDR(tmp), ++i)
        vec->values[i] = CAR(tmp);

    return x;
}

extern Sv
*Sv_new_structure(struct Stu *stu, Sv_type type, Sv *value_list)
{
    Sv_vector *field_vector = Type_field_vector(stu, type)->val.vector;

    Sv *x = Sv_new(stu, type);
    Sv **fields =
        CHECKED_MALLOC(field_vector->length * sizeof(Sv*));
    x->val.structure = fields;

    Sv *tmp = value_list;
    for (long i = 0; i < field_vector->length; ++i, tmp = CDR(tmp)) {
        if (tmp->type != SV_CONS)
            return Sv_new_err(stu, "Record argument is not a proper list");
        if (IS_NIL(tmp))
            return Sv_new_err(stu, "Not enough arguments for structure");
        fields[i] = CAR(tmp);
    }

    if (!IS_NIL(tmp))
        return Sv_new_err(stu, "Too many arguments for structure");
    return x;
}

extern Sv
*Sv_new_structure_constructor(struct Stu *stu, Sv_type type) {
    Sv *x = Sv_new(stu, SV_STRUCTURE_CONSTRUCTOR);
    x->val.structure_constructor = type;
    return x;
}

extern Sv
*Sv_new_structure_access(struct Stu *stu, Sv *structure, Sv *field) {
    Sv *x = Sv_new(stu, SV_STRUCTURE_ACCESS);
    if (field->type != SV_SYM)
        return Sv_new_err(stu, "Field access field is not a symbol");
    x->val.reg[SV_CAR_REG] = structure;
    x->val.reg[SV_CDR_REG] = field;
    return x;
}

extern Sv
*Sv_new_foreign(struct Stu *stu, void *o, Sv_foreign_destructor_t d)
{
    Sv *x = Sv_new(stu, SV_FOREIGN);
    x->val.foreign.obj = o;
    x->val.foreign.destructor = d;
    return x;
}

extern Sv
*Sv_new_regex(struct Stu *stu, const char *re, int icase)
{
    Sv *x = Sv_new(stu, SV_REGEX);
    x->val.re.icase = icase;
    int flags = icase ? REG_ICASE : 0;
    if (regcomp(&(x->val.re.compiled), re, (flags | REG_EXTENDED)) != 0)
        err(1, "Sv_new_re");
    if ((x->val.re.spec = strdup(re)) == NULL)
        err(1, "Sv_new_re");
    return x;
}

extern void
Sv_destroy(Stu *stu, Sv **sv)
{
    int i;

    if (sv && *sv) {
        switch ((*sv)->type) {
        case SV_ERR:
        case SV_STR:
            if ((*sv)->val.buf) {
                free((*sv)->val.buf);
                (*sv)->val.buf = NULL;
            }
            break;

        case SV_CONS:
            /* GC will clean up cons cells. */
            for (i = 0; i < SV_CONS_REGISTERS; i++)
                (*sv)->val.reg[i] = NULL;
            break;

        case SV_LAMBDA:
            /* GC will clean up environments and lists. */
            if ((*sv)->val.ufunc) {
                (*sv)->val.ufunc->env = NULL;
                (*sv)->val.ufunc->formals = NULL;
                (*sv)->val.ufunc->body = NULL;
                Alloc_release(stu->sv_ufunc_alloc, (*sv)->val.ufunc);
                (*sv)->val.ufunc = NULL;
            }
            break;

        case SV_SPECIAL:
            /* GC will clean up special body. */
            if ((*sv)->val.special) {
                (*sv)->val.special->body = NULL;
                Alloc_release(stu->sv_special_alloc, (*sv)->val.special);
                (*sv)->val.special = NULL;
            }
            break;

        case SV_NATIVE_FUNC:
            free((*sv)->val.func);
            (*sv)->val.func = NULL;
            break;

        case SV_NATIVE_CLOS:
            free((*sv)->val.clos);
            (*sv)->val.clos = NULL;
            break;

        case SV_VECTOR:
            free((*sv)->val.vector);
            (*sv)->val.vector = NULL;
            break;

        case SV_FOREIGN:
            if ((*sv)->val.foreign.destructor != NULL) {
                (*sv)->val.foreign.destructor((*sv)->val.foreign.obj);
                (*sv)->val.foreign.destructor = NULL;
            }
            (*sv)->val.foreign.obj = NULL;
            break;

        case SV_REGEX:
            regfree(&((*sv)->val.re.compiled));
            if ((*sv)->val.re.spec != NULL) {
                free((*sv)->val.re.spec);
                (*sv)->val.re.spec = NULL;
            }
            break;

        default:
            if ((*sv)->type >= SV_BUILTIN_TYPE_END) {
                free((*sv)->val.structure);
                (*sv)->val.structure = NULL;
            }
            break;
        }

        Alloc_release(stu->sv_alloc, *sv);
        *sv = NULL;
    }
}

static void
Sv_cons_dump(Stu *stu, Sv *sv, FILE *out)
{
    Sv *car = CAR(sv);
    Sv *cdr = CDR(sv);
    Sv_dump(stu, car, out);
    if (!IS_NIL(cdr)) {
        switch (cdr->type) {
        case SV_CONS:
            fprintf(out, " ");
            Sv_cons_dump(stu, cdr, out);
            break;

        default:
            fprintf(out, " . ");
            Sv_dump(stu, cdr, out);
            break;
        }
    }
}

extern void
Sv_dump(Stu *stu, Sv *sv, FILE *out)
{
    if (sv) {
        switch (sv->type) {
        case SV_SYM:
            fprintf(out, "%s", Symtab_get_name(stu, sv->val.i));
            break;

        case SV_ERR:
            if (sv->val.buf)
                fprintf(out, "<err \"%s\">", sv->val.buf);
            break;

        case SV_STR:
            if (sv->val.buf)
                fprintf(out, "\"%s\"", sv->val.buf);
            break;

        case SV_CONS:
            fprintf(out, "(");
            Sv_cons_dump(stu, sv, out);
            fprintf(out, ")");
            break;

        case SV_SPECIAL:
            switch (sv->val.special->type) {
            case SV_SPECIAL_COMMA:
                fprintf(out, ",");
                break;

            case SV_SPECIAL_COMMA_SPREAD:
                fprintf(out, ",@");
                break;

            case SV_SPECIAL_BACKQUOTE:
                fprintf(out, "`");
                break;
            }

            Sv_dump(stu, sv->val.special->body, out);
            break;

        case SV_INT:
            fprintf(out, "%ld", sv->val.i);
            break;

        case SV_FLOAT:
            fprintf(out, "%0.10f", sv->val.f);
            break;

        case SV_RATIONAL:
            fprintf(out, "%ld/%ld", sv->val.rational.n, sv->val.rational.d);
            break;

        case SV_BOOL:
            fprintf(out, "%s", sv->val.i ? "#t" : "#f");
            break;

        case SV_NATIVE_FUNC:
            fprintf(out, "<native-function>");
            break;

        case SV_NATIVE_CLOS:
            fprintf(out, "<native-closure>");
            break;

        case SV_LAMBDA:
            if (sv->val.ufunc) {
                PUSH_SCOPE(stu);
                Sv_dump(
                    stu, Sv_cons(
                        stu,
                        Sv_new_sym(stu, "λ"),
                        Sv_cons(
                            stu,
                            sv->val.ufunc->formals,
                            sv->val.ufunc->body)), out);
                POP_SCOPE(stu);
            }
            break;

        case SV_NIL:
            fprintf(out, "()");
            break;

        case SV_VECTOR:
            fprintf(out, "[");
            if (sv->val.vector->length > 0) {
                Sv_dump(stu, sv->val.vector->values[0], out);
                for (long i = 1; i < sv->val.vector->length; ++i) {
                    fprintf(out, " ");
                    Sv_dump(stu, sv->val.vector->values[i], out);
                }
            }
            fprintf(out, "]");
            break;

        case SV_STRUCTURE_CONSTRUCTOR:
            fprintf(out, "<constructor %s>", Type_name_string(stu, sv->type));
            break;

        case SV_STRUCTURE_ACCESS:
            Sv_dump(stu, sv->val.reg[SV_CAR_REG], out);
            fprintf(out, "::");
            Sv_dump(stu, sv->val.reg[SV_CDR_REG], out);
            break;

        case SV_FOREIGN:
            fprintf(out, "<foreign %p>", Sv_get_foreign_obj(stu, sv));
            break;

        case SV_REGEX:
            fprintf(out, "<regex #/%s/%s>", sv->val.re.spec, (sv->val.re.icase ? "i" : ""));
            break;

        default:
            fprintf(out, "<structure %s>", Type_name_string(stu, sv->type));
            break;
        }
    }
}

static Sv
*Sv_copy_sym(Stu *stu, Sv *x)
{
    Sv *y = Sv_new(stu, SV_SYM);
    y->val.i = x->val.i;
    return y;
}

static Sv
*Sv_copy_vector(Stu *stu, Sv *x)
{
    Sv_vector *vec = x->val.vector;
    size_t size = sizeof(Sv_vector) + vec->length * sizeof(Sv*);
    Sv_vector *vec_copy = CHECKED_MALLOC(size);
    memcpy(vec_copy, vec, size);
    Sv *copy = Sv_new(stu, SV_VECTOR);
    copy->val.vector = vec_copy;
    return copy;
}

extern Sv
*Sv_vector_append(Stu *stu, Sv *x, Sv *y)
{
    if (IS_NIL(y)) return x;

    Sv_vector *vec = x->val.vector;
    size_t new_size = sizeof(Sv_vector) + (vec->length + 1) * sizeof(Sv *);
    Sv_vector *new_vec = CHECKED_MALLOC(new_size);
    new_vec->length = vec->length + 1;
    memcpy(new_vec->values, vec->values, (vec->length * sizeof(Sv *)));
    new_vec->values[new_vec->length - 1] = y;

    Sv *z = Sv_new(stu, SV_VECTOR);
    z->val.vector = new_vec;

    return z;
}

extern Sv
*Sv_copy_structure(Stu *stu, Sv *x) {
    long length = Type_field_vector(stu, x->type)->val.vector->length;
    Sv *copy = Sv_new(stu, x->type);
    Sv **fields = CHECKED_MALLOC(length * sizeof(Sv*));
    copy->val.structure = fields;
    memcpy(fields, x->val.structure, length);
    return copy;
}


extern Sv
*Sv_copy(Stu *stu, Sv *x)
{
    int i;
    Sv *y = NULL;

    if (x) {
        switch (x->type) {
        case SV_NIL:
            y = x;
            break;

        case SV_SYM:
            y = Sv_copy_sym(stu, x);
            break;

        case SV_ERR:
            if (x->val.buf)
                y = Sv_new_err(stu, x->val.buf);
            break;

        case SV_STR:
            if (x->val.buf)
                y = Sv_new_str(stu, x->val.buf);
            break;

        case SV_CONS:
            y = Sv_new(stu, SV_CONS);
            for (i = 0; i < SV_CONS_REGISTERS; i++) {
                y->val.reg[i] = Sv_copy(stu, x->val.reg[i]);
            }
            break;

        case SV_SPECIAL:
            y = Sv_new_special(stu, x->val.special->type, x->val.special->body);
            break;

        case SV_INT:
            y = Sv_new_int(stu, x->val.i);
            break;

        case SV_FLOAT:
            y = Sv_new_float(stu, x->val.f);
            break;

        case SV_RATIONAL:
            y = Sv_new_rational(stu, x->val.rational.n, x->val.rational.d);
            break;

        case SV_BOOL:
            y = Sv_new_bool(stu, (short) x->val.i);
            break;

        case SV_LAMBDA:
            if (x->val.ufunc) {
                y = Sv_new_lambda(
                    stu, x->val.ufunc->env, x->val.ufunc->formals, x->val.ufunc->body);
            }
            break;

        case SV_VECTOR:
            y = Sv_copy_vector(stu, x);
            break;

        case SV_STRUCTURE_CONSTRUCTOR:
            y = Sv_new_structure_constructor(stu, x->val.structure_constructor);
            break;

        case SV_REGEX:
            y = Sv_new_regex(stu, x->val.re.spec, x->val.re.icase);
            break;

        default:
            y = Sv_copy_structure(stu, x);
            break;
        }
    }

    return y;
}

extern Sv
*Sv_cons(Stu *stu, Sv *x, Sv *y)
{
    Sv *z = Sv_new(stu, SV_CONS);

    z->val.reg[SV_CAR_REG] = x;
    z->val.reg[SV_CDR_REG] = y;

    return z;
}

extern Sv
*Sv_reverse(Stu *stu, Sv *x)
{
    Sv *y = NIL;

    while (!IS_NIL(x) && x->type == SV_CONS) {
        y = Sv_cons(stu, CAR(x), y);
        x = CDR(x);
    }

    return y;
}

extern Sv
*Sv_list(Stu *stu, Sv *x)
{
    Sv *y = NULL, *z = NIL;

    if (!x)
        return x;

    if (x->type != SV_CONS)
        return Sv_cons(stu, x, NIL);

    while (!IS_NIL(x) && (y = CAR(x))) {
        z = Sv_cons(stu, y, z);
        x = CDR(x);
    }

    return Sv_reverse(stu, z);
}

extern Sv
*Sv_expand_1(Stu *stu, Sv *x)
{
    if (!x || x->type != SV_CONS)
        return x;

    Sv *head = CAR(x);
    Sv *macro;

    if (head->type != SV_SYM)
        return x;

    macro = Env_main_get(stu, head);
    if (!IS_MACRO(macro))
        return x;

    return Sv_call(stu, stu->main_env, macro, CDR(x));
}

extern Sv
*Sv_expand(Stu *stu, Sv *x)
{
    Sv *head;

    do {
        x = Sv_expand_1(stu, x);
        head = CAR(x);
    } while (IS_MACRO(Env_main_get(stu, head)));

    return x;
}

extern void
*Sv_get_foreign_obj(Stu *stu, Sv *x)
{
    return x->type == SV_FOREIGN ? x->val.foreign.obj : NULL;
}

static Sv
*Sv_eval_vector(Stu *stu, Env *env, Sv *x)
{
    Sv *copy = Sv_copy(stu, x);
    Sv_vector *vec = copy->val.vector;
    for (long i = 0; i < vec->length; ++i)
        vec->values[i] = Sv_eval(stu, env, vec->values[i]);
    return copy;
}

static Sv
*Sv_eval_structure_access(Stu *stu, Env *env, Sv *x)
{
    Sv *structure = Sv_eval(stu, env, x->val.reg[SV_CAR_REG]);
    if (IS_NIL(structure) || structure->type == SV_ERR) {
        return structure;
    }
    Sv *field = x->val.reg[SV_CDR_REG];
    return structure->val.structure[Type_field_index(stu, structure->type, field)];
}

extern Sv
*Sv_eval(Stu *stu, Env *env, Sv *x)
{
    Sv *y = NULL;

    if (!x)
        return x;

    PUSH_SCOPE(stu);
    switch (x->type) {
    case SV_SYM:
        /*
         * If the symbol exists but it's value is NULL, then it is
         * the empty list.
         */
        if ((y = Env_get(env, x)) == NULL && !Env_exists(env, x)) {
            y = Sv_new_err(stu, "possibly unknown symbol");
        }
        break;

    case SV_SPECIAL:
        y = Sv_eval_special(stu, env, x);
        break;

    case SV_CONS:
        y = Sv_eval_sexp(stu, env, x);
        break;

    case SV_VECTOR:
        y = Sv_eval_vector(stu, env, x);
        break;

    case SV_STRUCTURE_ACCESS:
        y = Sv_eval_structure_access(stu, env, x);
        break;

    default:
        y = x;
        break;
    }
    POP_N_SAVE(stu, y);

    return y;
}

extern Sv
*Sv_eval_list(Stu *stu, Env *env, Sv *x, Env **new_env)
{
    if (IS_NIL(x)) return x;

    Env *next_env = env;
    Sv *last_result = NIL;
    Sv *cur = CAR(x);

    Env *tail, *head;
    Env_capture_save(stu, &tail, &head);
    PUSH_N_SAVE(stu, head);

    while (!IS_NIL(cur)) {
        Env_capture_reset(stu);
        last_result = Sv_eval(stu, next_env, cur);
        next_env = Env_capture_rebase(stu, next_env);
        SCOPE_SAVE(stu, next_env);

        x = CDR(x);
        cur = CAR(x);
    }

    Env_capture_restore(stu, tail, head);
    POP_N_SAVE(stu, last_result);

    /* Save updated environment in the new head scope stack. */
    if (new_env != NULL) {
        SCOPE_SAVE(stu, next_env);
        *new_env = next_env;
    }

    return last_result;
}

extern Sv
*Sv_eval_special(Stu *stu, Env *env, Sv *x)
{
    Sv_special *special = x->val.special;
    Sv *body = x->val.special->body;

    switch (special->type) {
    case SV_SPECIAL_BACKQUOTE:
        if (body->type == SV_SYM) {
            return Sv_eval_sexp(stu, env,
                Sv_cons(stu, Sv_new_sym(stu, "quote"), Sv_cons(stu, body, NIL)));
        } else if (body->type == SV_CONS) {
            return Sv_eval_special_cons(stu, env, body);
        } else {
            return body;
        }
        break;

    case SV_SPECIAL_COMMA:
        return Sv_new_err(stu, "Comma can be used only inside backquoted list");

    case SV_SPECIAL_COMMA_SPREAD:
        return Sv_new_err(stu, "Comma spread can be used only inside backquoted list");
    }

    return Sv_new_err(stu, "Troubles with quoting magic");
}

extern Sv
*Sv_eval_special_cons(Stu *stu, Env *env, Sv *x)
{
    if (IS_NIL(x))
        return x;

    Sv *head = CAR(x);

    if (head->type == SV_SPECIAL) {
        Sv_special *special = head->val.special;

        switch (special->type) {
        case SV_SPECIAL_COMMA:
            return Sv_cons(
                stu, Sv_eval(stu, env, special->body),
                Sv_eval_special_cons(stu, env, CDR(x)));

        case SV_SPECIAL_COMMA_SPREAD:
            if (special->body->type == SV_SYM || special->body->type == SV_CONS) {
                Sv *val = Sv_eval(stu, env, special->body);
                if (!IS_NIL(val) && val->type == SV_CONS) {
                    return val;
                } else {
                    return Sv_new_err(stu, "spread operator applied to atom");
                }
            } else {
                return Sv_new_err(stu, "spread operator can be applied only to symbol");
            }
            break;

        case SV_SPECIAL_BACKQUOTE:
            return Sv_new_err(stu, "backquote is not permitted inside of other backquote");
        }
    } else if (head->type == SV_CONS)  {
        return Sv_cons(
            stu, Sv_eval_special_cons(stu, env, head), Sv_eval_special_cons(stu, env, CDR(x)));
    } else {
        return Sv_cons(stu, head, Sv_eval_special_cons(stu, env, CDR(x)));
    }

    return Sv_new_err(stu, "Got confused inside of backquoted list");
}

extern Sv
*Sv_eval_sexp(Stu *stu, Env *env, Sv *x)
{
    Sv *cur = NULL, *y = NULL, *z = NULL;
    Special_form_f special = NULL;
    cur = x = Sv_expand(stu, x);

    if (x->type != SV_CONS)
        return Sv_eval(stu, env, x);

    z = CAR(cur);

    if (z->type == SV_SYM && ((special = Special_form_get_f(stu, z)) != NULL)) {
        Call_stack_push(stu, z);
        y = special(stu, env, CDR(cur));
        Call_stack_pop(stu);
        return y;
    }

    /* Evaluate all arguments. */
    while (!IS_NIL(cur)) {
        if (cur->type == SV_CONS) {
            y = Sv_cons(stu, Sv_eval(stu, env, CAR(cur)), y);
            cur = CDR(cur);
        } else {
            y = Sv_cons(stu, cur, y);
            break;
        }
    }
    x = Sv_reverse(stu, y);
    y = CAR(x);

    /* The car should now be a function. */
    if (y)
        switch (y->type) {
        case SV_NATIVE_FUNC:
        case SV_NATIVE_CLOS:
        case SV_LAMBDA:
        case SV_STRUCTURE_CONSTRUCTOR:
            if (z->type == SV_SYM) {
                Call_stack_push(stu, z);
            } else {
                Call_stack_push(stu, y);
            }
            z = Sv_call(stu, env, y, CDR(x));
            Call_stack_pop(stu);
            return z;

        default:
            break;
        }

    return Sv_new_err(stu, "first element is not a function");
}

extern Sv
*Sv_call(Stu *stu, Env *env, Sv *f, Sv *a)
{
    short varargs = 0;
    Env *call_env;
    Sv *formals, *formal, *arg, *partial, *args = a;
    formals = formal = arg = partial = NULL;

    if (!f)
        return f;

    if (f->type == SV_NATIVE_FUNC)
        return Sv_native_func_call(stu, env, f->val.func, a);

    if (f->type == SV_NATIVE_CLOS)
        return Sv_native_closure_call(stu, env, f->val.clos, a);

    if (f->type == SV_STRUCTURE_CONSTRUCTOR)
        return Sv_new_structure(stu, f->val.structure_constructor, a);

    if (f->type == SV_LAMBDA) {
        /* Bind the formals to the arguments. */
        formals = f->val.ufunc->formals;
        call_env = f->val.ufunc->env;

        while (!IS_NIL(formals) && (formal = CAR(formals))) {
            if (!varargs && formal->type == SV_SYM && formal->val.i == Symtab_get_id(stu, "&")) {
                varargs = 1;
            } else {
                /* Get the next arg. */
                if (args && (arg = CAR(args))) {
                    if (varargs) {
                        call_env = Env_put(stu, call_env, formal, Sv_list(stu, args));
                        break;
                    } else {
                        call_env = Env_put(stu, call_env, formal, arg);
                    }
                } else if (varargs) {
                    /* Variable args not supplied. */
                    call_env = Env_put(stu, call_env, formal, NULL);
                    break;
                } else {
                    /* No more arguments to consume. */
                    partial = Sv_copy(stu, f);
                    partial->val.ufunc->formals = formals;
                    partial->val.ufunc->env = call_env;
                    break;
                }
                args = CDR(args);
            }
            formals = CDR(formals);
        }

        if (partial) {
            return partial;
        } else {
            return Sv_eval_list(stu, call_env, f->val.ufunc->body, NULL);
        }
    }

    return Sv_new_err(stu, "can only call functions");
}
