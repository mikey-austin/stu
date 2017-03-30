#include <stdlib.h>
#include <err.h>
#include <string.h>

#include "stu.h"
#include "hash.h"
#include "symtab.h"

extern void
Symtab_init(Stu *stu)
{
    stu->sym_name_to_id = Hash_new(NULL);
    if ((stu->sym_id_to_name = calloc(stu->sym_num_ids, sizeof(*(stu->sym_id_to_name)))) == NULL)
        err(1, "Symtab_init");
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
            if ((stu->sym_id_to_name = realloc(
                stu->sym_id_to_name,
                stu->sym_num_ids * sizeof(*stu->sym_id_to_name))) == NULL)
            {
                err(1, "Symtab_get_id");
            }
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
