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

#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "stu_private.h"
#include "symtab.h"
#include "utils.h"

extern void
Symtab_init(Stu *stu)
{
    stu->sym_name_to_id = Hash_new(NULL);
    stu->sym_id_to_name = CHECKED_CALLOC(
        stu->sym_num_ids,
        sizeof(*(stu->sym_id_to_name)));
}

extern
void Symtab_destroy(Stu *stu)
{
    long i;

    if (stu->sym_id_to_name) {
        for (i = 0; i < stu->sym_name_to_id->num_entries; i++)
            free(stu->sym_id_to_name[i]);
        free(stu->sym_id_to_name);
        stu->sym_id_to_name = NULL;
    }
    Hash_destroy(&(stu->sym_name_to_id));
}

extern long
Symtab_get_id(Stu *stu, const char *name)
{
    long id = 0;
    Hash_ent *mapping = Hash_get(stu->sym_name_to_id, name);

    if (mapping == NULL) {
        /* We haven't seen this symbol yet. */
        id = stu->sym_name_to_id->num_entries;
        Hash_put(stu->sym_name_to_id, name, (void *) id);

        if (stu->sym_name_to_id->num_entries > stu->sym_num_ids) {
            stu->sym_num_ids *= 2;
            stu->sym_id_to_name = CHECKED_REALLOC(
                stu->sym_id_to_name,
                stu->sym_num_ids * sizeof(*stu->sym_id_to_name));
        }
        stu->sym_id_to_name[id] = strdup(name);
    } else {
        id = (long) mapping->v;
    }

    return id;
}

extern char
*Symtab_get_name(Stu *stu, long id)
{
    return (id >= 0 && id < stu->sym_name_to_id->num_entries)
        ? stu->sym_id_to_name[id]
        : NULL;
}
