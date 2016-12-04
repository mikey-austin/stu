#include <stdlib.h>
#include <err.h>
#include <string.h>

#include "hash.h"
#include "symtab.h"

static Hash *name_to_id = NULL;
static char **id_to_name = NULL;
static long num_ids = HASH_SIZE;

extern void
Symtab_init(void)
{
    name_to_id = Hash_new(NULL);
    if ((id_to_name = calloc(num_ids, sizeof(*id_to_name))) == NULL)
        err(1, "Symtab_init");

    /* Make sure nil is first with an id of zero. */
    Symtab_get_id("nil");
}

extern long
Symtab_get_id(const char *name)
{
    long id = 0;
    Hash_ent *mapping = Hash_get(name_to_id, name);

    if (mapping == NULL) {
        /* We haven't seen this symbol yet. */
        id = name_to_id->num_entries;
        Hash_put(name_to_id, name, (void *) id);

        if (name_to_id->num_entries > num_ids) {
            num_ids *= 2;
            if ((id_to_name = realloc(id_to_name, num_ids * sizeof(*id_to_name))) == NULL)
                err(1, "Symtab_get_id");
        }
        id_to_name[id] = strdup(name);
    } else {
        id = (long) mapping->v;
    }

    return id;
}

extern char
*Symtab_get_name(long id)
{
    return (id >= 0 && id < name_to_id->num_entries)
        ? id_to_name[id]
        : NULL;
}

extern
void Symtab_destroy(void)
{
    if (id_to_name) {
        for (long i = 0; i < name_to_id->num_entries; i++)
            free(id_to_name[i]);
        free(id_to_name);
        id_to_name = NULL;
    }
    Hash_destroy(&name_to_id);
}
