/*
 * Copyright (c) 2018 Mikey Austin <mikey@jackiemclean.net>
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
#include <unistd.h>

#include "stu_private.h"
#include "env.h"
#include "sv.h"
#include "builtins.h"
#include "gc.h"

static char
*append_file_to_loc(const char *location, const char *path)
{
    char buf[strlen(location) + strlen(path) + 2];

    buf[0] = '\0';
    strcat(buf, location);
    strcat(buf, "/");
    strcat(buf, path);

    return strdup(buf);
}

static char
*resolve_path(Stu *stu, char *file)
{
    size_t len;
    if (file == NULL || (len = strlen(file)) == 0) {
        return NULL;
    }

    if (file[0] == '/' || access(file, F_OK | R_OK) == 0) {
        return strdup(file);
    }

    char *next = NULL, *result = NULL;
    Sv *locations = stu->mod_include_locations;
    for (int i = 0; i < locations->val.vector->length; i++) {
        Sv *loc = locations->val.vector->values[i];
        next = append_file_to_loc(loc->val.buf, file);
        if (next == NULL) {
            break;
        } else if (access(next, F_OK | R_OK) == 0) {
            result = next;
            break;
        } else {
            free(next);
            next = NULL;
        }
    }

    return result;
}

extern Sv
*Mod_import_from_file(Stu *stu, Env *base, Sv *file)
{
    if (file->type != SV_STR) {
        return Sv_new_err(stu, "import needs a string file path");
    }

    char *resolved_path = resolve_path(stu, file->val.buf);
    if (!resolved_path) {
        return Sv_new_err(stu, "could not resolve file path");
    }

    Sv *forms = Stu_parse_file(stu, resolved_path);
    free(resolved_path);

    /*
     * TODO: Make a way to pull the type name from out of a module
     *       file's list of forms.
     */
    Sv *name = file;

    PUSH_SCOPE(stu);

    Env *captured;
    Sv_eval_list(stu, base, forms, &captured);
    if (base == captured) {
        return Sv_new_err(stu, "Cannot import module; nothing bound within");
    }

    Sv *fields = NIL, *vals = NIL;
    for (Env *cur = captured; cur != base; cur = cur->prev) {
        fields = Sv_cons(stu, Sv_new_sym_from_id(stu, cur->sym), fields);
        vals = Sv_cons(stu, cur->val, vals);
    }

    Sv_type type = Type_new(stu, name, Sv_new_vector(stu, fields));
    Sv *constructor = Sv_new_structure_constructor(stu, type);
    Sv *module = Sv_eval(stu, base, Sv_cons(stu, constructor, vals));

    POP_N_SAVE(stu, module);

    return module;
}
