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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "alloc/alloc.h"
#include "env.h"
#include "builtins.h"
#include "gc.h"
#include "sv.h"
#include "svlist.h"
#include "lexer.h"
#include "parser.h"
#include "special_form.h"
#include "symtab.h"
#include "hash.h"
#include "stu_private.h"
#include "utils.h"

#define VALIDATOR_STACK_SIZE 256
#define FORM_VALID    1
#define FORM_INVALID  0
#define FORM_PARTIAL -1

/*
 * NIL singleton object. This object is shared among multiple
 * stu interpreter instances.
 */
Sv *Sv_nil = NULL;
int Sv_interpreters = 0;

extern Stu
*Stu_new(void)
{
    enum Alloc_type default_alloc;
    Stu *stu = CHECKED_CALLOC(1, sizeof(*stu));;

    /* Initialize allocators. */
#ifdef ALLOC_SYSTEM
    default_alloc = ALLOC_TYPE_SYS;
#elif ALLOC_SLAB
    default_alloc = ALLOC_TYPE_SLAB;
#else
#  error "no memory allocator defined!"
#endif
    stu->sv_alloc = Alloc_new(stu, sizeof(Sv), default_alloc);
    stu->env_alloc = Alloc_new(stu, sizeof(Env), default_alloc);
    stu->sv_special_alloc = Alloc_new(stu, sizeof(Sv_special), default_alloc);
    stu->sv_ufunc_alloc = Alloc_new(stu, sizeof(Sv_ufunc), default_alloc);
    stu->gc_scope_alloc = Alloc_new(stu, sizeof(Scope), default_alloc);

    /* Initialize interpreter. */
    stu->gc_scope_stack = NULL;
    stu->gc_head = NULL;
    stu->gc_tail = NULL;
    stu->gc_allocs = 0;

    /* Initialize exception handling. */
    stu->last_try_marker = NULL;
    stu->last_exception = NIL;

    stu->stats_gc_managed_objects = 0;
    stu->stats_gc_collections = 0;
    stu->stats_gc_frees = 0;
    stu->stats_gc_allocs = 0;
    stu->stats_gc_cleaned = 0;
    stu->stats_gc_scope_pushes = 0;
    stu->stats_gc_scope_pops = 0;

    stu->main_env = NULL;
    stu->sym_name_to_id = NULL;
    stu->sym_id_to_name = NULL;
    stu->sym_num_ids = HASH_SIZE;

    stu->native_func_args = NULL;
    stu->native_func_args_capacity = 0;

    Type_registry_init(&stu->type_registry);

    PUSH_SCOPE(stu);
    Symtab_init(stu);

    stu->call_stack = NIL;

    /* Setup NIL singleton outside of allocators and GC. */
    if (Sv_nil == NULL) {
        Sv_nil = CHECKED_CALLOC(1, sizeof(*Sv_nil));
        Sv_nil->type = SV_NIL;
    }
    Env_main_put(stu, Sv_new_sym(stu, "nil"), Sv_nil);
    Sv_interpreters += 1;

    /* Ensure nil and all special form symbols get the ids we expect */
    if (Symtab_get_id(stu, "nil") != 0)
        err(1, "stu_new");
    Special_form_register_symbols(stu);

    Builtin_init(stu);
    POP_SCOPE(stu);

    return stu;
}

extern void
Stu_destroy(Stu **stu)
{
    Stu *s;

    if (!stu)
        return;

    s = *stu;
    if (s) {
        Sv_interpreters -= 1;
        if (Sv_interpreters <= 0) {
            free(Sv_nil);
            Sv_nil = NULL;
            Sv_interpreters = 0;
        }
        Symtab_destroy(s);
        Gc_sweep(s, 1);
        Alloc_destroy(&(s->sv_alloc));
        Alloc_destroy(&(s->env_alloc));
        Alloc_destroy(&(s->sv_special_alloc));
        Alloc_destroy(&(s->sv_ufunc_alloc));
        Alloc_destroy(&(s->gc_scope_alloc));
        Type_registry_release(&s->type_registry);
        free(s->native_func_args);
        free(s);
    }

    *stu = NULL;
}

extern Svlist
*Stu_parse_file(Stu *stu, const char *file)
{
    Svlist *list = Svlist_new();

    if (!file || !strcmp(file, "-")) {
        yyin = stdin;
    } else {
        if ((yyin = fopen(file, "r")) == NULL)
            err(1, "Parse_file");
    }

    switch (yyparse(stu, &list)) {
    case 2:
        errx(1, "Parser memory allocation error");
        break;
    }

    if (yyin && yyin != stdin)
        fclose(yyin);
    yylex_destroy();

    return list;
}

extern Sv
*Stu_eval_file(Stu *stu, const char *file)
{
    Svlist *forms;
    Sv *result;

    PUSH_SCOPE(stu);
    forms = Stu_parse_file(stu, file);
    result = Svlist_eval(stu, stu->main_env, forms);
    Gc_lock(stu, (Gc *) result);
    Svlist_destroy(&forms);
    POP_SCOPE(stu);

    return result;
}

extern Svlist
*Stu_parse_buf(Stu *stu, const char *buf)
{
    Svlist *list = Svlist_new();

    if (buf) {
        YY_BUFFER_STATE bp = yy_scan_string(buf);
        yy_switch_to_buffer(bp);
        switch (yyparse(stu, &list)) {
        case 2:
            errx(1, "Parser memory allocation error");
            break;
        }
        yy_delete_buffer(bp);
    }

    return list;
}

extern Sv
*Stu_eval_buf(Stu *stu, const char *buf)
{
    Svlist *forms;
    Sv *result;

    PUSH_SCOPE(stu);
    forms = Stu_parse_buf(stu, buf);
    result = Svlist_eval(stu, stu->main_env, forms);
    Gc_lock(stu, (Gc *) result);
    Svlist_destroy(&forms);
    POP_SCOPE(stu);

    return result;
}

extern int
Stu_is_valid_form(Stu *stu, const char *buf)
{
    int valid = FORM_VALID, tok = 0, top = -1;
    char stack[VALIDATOR_STACK_SIZE], opposite;

    if (buf) {
        YY_BUFFER_STATE bp = yy_scan_string(buf);
        yy_switch_to_buffer(bp);
        while (tok = yylex()) {
            switch (tok) {
            case '(':
            case '[':
                if (++top >= VALIDATOR_STACK_SIZE) {
                    warnx("cannot validate nested forms > %d levels",
                          VALIDATOR_STACK_SIZE);
                    valid = FORM_INVALID;
                    goto done;
                }
                stack[top] = tok;
                break;

            case ')':
            case ']':
                opposite = tok == ')'
                    ? '('
                    : '[';
                if (top < 0 || stack[top--] != opposite) {
                    warnx("form contains unbalanced %c", (char) tok);
                    valid = FORM_INVALID;
                    goto done;
                }
                break;
            }
        }

    done:
        yy_delete_buffer(bp);
    }

    if (valid && top >= 0) {
        /* This is valid *so far*, but is incomplete. */
        valid = FORM_PARTIAL;
    }

    return valid;
}

extern void
Stu_release_val(Stu *stu, Sv *sv)
{
    Gc_unlock(stu, (Gc *) sv);
}

extern void
Stu_dump_val(Stu *stu, Sv *sv, FILE *out)
{
    PUSH_N_SAVE(stu, sv);
    Sv_dump(stu, sv, out);
    POP_SCOPE(stu);
}

extern void
Stu_dump_stats(Stu *stu, FILE *out)
{
    PUSH_SCOPE(stu);
    Gc_dump_stats(stu, out);
    POP_SCOPE(stu);
}
