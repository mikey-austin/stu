#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "hash.h"

static Hash_ent *Hash_find_entry(Hash *hash, const char *key);
static void Hash_resize(Hash *hash, int new_size);
static unsigned int Hash_lookup(const char *key);
static void Hash_create_entries(Hash *hash);
static void Hash_default_destroy(Hash_ent *entry);

extern Hash
*Hash_new(int size, void (*destroy)(Hash_ent *entry))
{
    Hash *new = NULL;

    if ((new = malloc(sizeof(*new))) == NULL)
        err(1, "Hash_create");

    new->size = size;
    if (destroy)
        new->destroy = destroy;
    else
        new->destroy = Hash_default_destroy;

    /* Leave the entries uninitialized until the first insert. */
    new->entries     = NULL;
    new->num_entries = 0;

    return new;
}

extern void
Hash_destroy(Hash **hash)
{
    if (hash == NULL || *hash == NULL)
        return;

    if ((*hash)->destroy && (*hash)->num_entries > 0) {
        for (int i = 0; i < (*hash)->size; i++) {
            (*hash)->destroy(((*hash)->entries + i));
        }
    }

    if (*hash) {
        if ((*hash)->entries) {
            free((*hash)->entries);
            (*hash)->entries = NULL;
        }
        free(*hash);
        *hash = NULL;
    }
}

extern void
Hash_reset(Hash *hash)
{
    if (hash->num_entries == 0)
        return;

    for (int i = 0; i < hash->size; i++) {
        hash->destroy((hash->entries + i));
        hash->entries[i].v = NULL;
    }

    /* Reset the number of entries counter. */
    hash->num_entries = 0;
}

extern void
Hash_put(Hash *hash, const char *key, void *value)
{
    Hash_ent *entry;

    if (key == NULL)
        return;

    if (hash->entries == NULL) {
        Hash_create_entries(hash);
    }
    else if (hash->num_entries >= hash->size) {
        Hash_resize(hash, (2 * hash->size));
    }

    entry = Hash_find_entry(hash, key);
    if (entry->v != NULL) {
        /* An entry exists, clear contents before overwriting. */
        hash->destroy(entry);
    } else {
        /* As nothing was over written, increment the number of entries. */
        hash->num_entries++;
    }

    /* Setup the new entry. */
    sstrncpy(entry->k, key, MAX_KEY_LEN);
    entry->v = value;
}

extern void
*Hash_get(Hash *hash, const char *key)
{
    Hash_ent *entry;

    if (hash->entries == NULL || key == NULL)
        return NULL;

    entry = Hash_find_entry(hash, key);
    return entry->v;
}

extern int
Hash_exists(Hash *hash, const char *key)
{
    Hash_ent *entry;

    if (hash->entries == NULL || key == NULL)
        return 0;

    entry = Hash_find_entry(hash, key);
    return !strcmp(key, entry->k);
}

extern void
Hash_del(Hash *hash, const char *key)
{
    Hash_ent *entry;

    if (hash->entries == NULL)
        return;

    entry = Hash_find_entry(hash, key);
    if (entry->v != NULL) {
        hash->destroy(entry);
        *(entry->k) = '\0';
        entry->v    = NULL;
        hash->num_entries--;
    }
}

extern int
Hash_entries(Hash *hash, Hash_ent ***entries)
{
    Hash_ent *entry;
    Hash_ent **new_entries = NULL;
    int num_entries = 0;

    if (hash && (num_entries = hash->num_entries) > 0) {
        if ((new_entries = calloc(hash->num_entries, sizeof(*new_entries))) == NULL)
            err(1, "Hash_keys");
        *entries = new_entries;

        for (int i = 0, j = 0; i < hash->size && j < num_entries; i++) {
            entry = hash->entries + i;
            if(entry->k) {
                new_entries[j++] = entry;
            }
        }
    }

    return num_entries;
}

static Hash_ent
*Hash_find_entry(Hash *hash, const char *key)
{
    unsigned int i, j;
    Hash_ent *curr;

    i = j = (Hash_lookup(key) % hash->size);
    do {
        curr = hash->entries + i;
        if ((strcmp(key, curr->k) == 0) || (curr->v == NULL))
            break;

        i = ((i + 1) % hash->size);
    } while (i != j);

    return curr;
}

static void
Hash_resize(Hash *hash, int new_size)
{
    int i, old_size = hash->size;
    Hash_ent *old_entries = hash->entries;

    if (new_size <= hash->size) {
        warnx("Refusing to resize a hash of %d elements to %d",
              hash->size, new_size);
        return;
    }

    /*
     * We must create a whole new larger area and re-hash each element as
     * the hash function depends on the size, which is changing. This is
     * not very efficient for large hashes, so best to choose an
     * appropriate starting size.
     */
    hash->entries = (Hash_ent*) calloc(
        new_size, sizeof(*(hash->entries)));

    if (!hash->entries)
        errx(1, "Could not resize hash entries of size %d to %d",
             hash->size, new_size);

    /* Re-initialize the hash entries. */
    hash->size = new_size;
    hash->num_entries = 0;

    /* For each non-NULL entry, re-hash into the new entries array. */
    for (i = 0; i < old_size; i++) {
        if(old_entries[i].v != NULL) {
            Hash_put(hash, old_entries[i].k, old_entries[i].v);
        }
    }

    /* Cleanup the old entries. */
    free(old_entries);
    old_entries = NULL;
}

static void
Hash_create_entries(Hash *hash)
{
    hash->num_entries = 0;
    hash->entries = (Hash_ent*) calloc(
        hash->size, sizeof(*(hash->entries)));
    if (!hash->entries)
        err(1, "Could not allocate hash entries of size %d", hash->size);
}

/*
 * Use the djb2 string hash function.
 */
static unsigned int
Hash_lookup(const char *key)
{
    int c;
    unsigned int hash = 5381;

    while((c = *key++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

static void
Hash_default_destroy(Hash_ent *entry)
{
    if (entry)
        entry->v = NULL;
}
