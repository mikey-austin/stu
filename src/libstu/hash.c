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

#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "hash.h"

unsigned int
hash_key(const char *key)
{
    int c;
    unsigned int h = 5381;

    while((c = *key++))
        h = ((h << 5) + h) + c;

    return h % HASH_SIZE;
}

extern Hash
*Hash_new(void (*destroy)(Hash_ent *entry))
{
    Hash *new = NULL;

    if ((new = calloc(1, sizeof(*new))) == NULL)
        err(1, "Hash_new");
    new->destroy = destroy;

    return new;
}

extern void
Hash_destroy(Hash **to_destroy)
{
    Hash *hash = NULL;
    Hash_ent *cur = NULL, *next = NULL;

    if (to_destroy == NULL || (hash = *to_destroy) == NULL)
        return;

    for (cur = hash->entries[HEAD]; cur; cur = next) {
        next = NEXT_ENTRY(cur);
        if (hash->destroy)
            hash->destroy(cur);
        free(cur);
    }

    free(hash);
    *to_destroy = NULL;
}

extern void
Hash_put(Hash *hash, const char *key, void *value)
{
    Hash_ent *new;

    if (key == NULL)
        return;

    Hash_del(hash, key);
    hash->num_entries++;
    if ((new = calloc(1, sizeof(*new))) == NULL)
        err(1, "Hash_put");

    sstrncpy(new->k, key, MAX_KEY_LEN);
    new->v = value;

    /* Add to the end of the entries list.  */
    if (hash->entries[HEAD] == NULL) {
        hash->entries[HEAD] = hash->entries[TAIL] = new;
    } else {
        new->entries[NEXT] = hash->entries[TAIL];
        hash->entries[TAIL]->entries[PREV] = new;
        hash->entries[TAIL] = new;
    }

    /* Finally add to the relevant bucket. */
    long i = hash_key(key);
    if (hash->buckets[i] == NULL) {
        hash->buckets[i] = new;
    } else {
        /* Collision, prepend new entry. */
        new->collisions[PREV] = hash->buckets[i];
        hash->buckets[i]->collisions[NEXT] = new;
        hash->buckets[i] = new;
    }
}

extern Hash_ent
*Hash_get(Hash *hash, const char *key)
{
    Hash_ent *ent = NULL;

    if (key == NULL)
        return NULL;

    long i = hash_key(key);
    for (ent = hash->buckets[i]; ent; ent = ent->collisions[PREV]) {
        if (!strcmp(ent->k, key))
            break;
    }

    return ent;
}

extern void
Hash_del(Hash *hash, const char *key)
{
    Hash_ent *ent;

    ent = Hash_get(hash, key);
    if (ent != NULL) {
        hash->num_entries--;
        if (hash->destroy)
            hash->destroy(ent);

        /* Remove from entries list. */
        if (ent->entries[PREV] == NULL)
            hash->entries[TAIL] = ent->entries[NEXT];
        else
            ent->entries[PREV]->entries[NEXT] = ent->entries[NEXT];

        if (ent->entries[NEXT] == NULL)
            hash->entries[HEAD] = ent->entries[PREV];
        else
            ent->entries[NEXT]->entries[PREV] = ent->entries[PREV];

        /* Remove from collisions list. */
        long i = hash_key(key);
        if (ent->collisions[PREV] != NULL)
            ent->collisions[PREV]->collisions[NEXT] = ent->collisions[NEXT];

        if (ent->collisions[NEXT] == NULL)
            hash->buckets[i] = ent->collisions[PREV];
        else
            ent->collisions[NEXT]->collisions[PREV] = ent->collisions[PREV];

        free(ent);
    }
}

extern Hash_ent
*Hash_entries(Hash *hash)
{
    return hash->entries[HEAD];
}
