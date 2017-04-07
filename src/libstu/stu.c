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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "builtins.h"
#include "gc.h"
#include "sv.h"
#include "svlist.h"
#include "lexer.h"
#include "parser.h"
#include "symtab.h"
#include "hash.h"
#include "stu.h"

/*
 * NIL singleton object. This object is shared among multiple
 * stu interpreter instances.
 */
Sv *Sv_nil = NULL;

extern Stu
*Stu_new(void)
{
    Stu *stu = NULL;
    if ((stu = calloc(1, sizeof(*stu))) == NULL) {
        err(1, "Stu_new");
    }

    /* Initialize interpreter. */
    stu->gc_scope_stack = NULL;
    stu->gc_stack_size = 0;
    stu->max_gc_stack_size = 20;

    stu->gc_head = NULL;
    stu->gc_tail = NULL;
    stu->gc_allocs = 0;

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

    PUSH_SCOPE(stu);
    Symtab_init(stu);

    /* Setup NIL singleton. */
    if (Sv_nil == NULL)
        Sv_nil = Sv_new(stu, SV_NIL);
    Env_main_put(stu, Sv_new_sym(stu, "nil"), Sv_nil);

    /* Make sure nil is the first symbol with an id of zero. */
    Symtab_get_id(stu, "nil");

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
        Symtab_destroy(s);
        Gc_sweep(s, 0);
        free(s);
    }

    *stu = NULL;
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
    Svlist_destroy(&forms);
    POP_SCOPE(stu);

    return result;
}

extern Sv
*Stu_eval_buf(Stu *stu, const char *buf)
{
    Svlist *forms;
    Sv *result;

    PUSH_SCOPE(stu);
    forms = Stu_parse_buf(stu, buf);
    result = Svlist_eval(stu, stu->main_env, forms);
    Svlist_destroy(&forms);
    POP_SCOPE(stu);

    return result;
}

extern void
Stu_dump_sv(Stu *stu, Sv *sv, FILE *out)
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
