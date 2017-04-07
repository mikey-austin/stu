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

#ifndef HASH_DEFINED
#define HASH_DEFINED

#define MAX_KEY_LEN 255
#define HASH_SIZE   97
#define PREV        0
#define NEXT        1
#define HEAD        0
#define TAIL        1

#define NEXT_ENTRY(e)       ((e) ? (e)->entries[PREV] : NULL)
#define HASH_NUM_ENTRIES(h) ((h) ? (h)->num_entries : 0)

struct Hash_ent;
typedef struct Hash_ent {
    struct Hash_ent *entries[2];
    struct Hash_ent *collisions[2];
    char  k[MAX_KEY_LEN + 1];
    void *v;
} Hash_ent;

typedef struct Hash {
    int  num_entries;
    void (*destroy)(struct Hash_ent *entry);
    struct Hash_ent *entries[2];
    struct Hash_ent *buckets[HASH_SIZE];
} Hash;

extern Hash *Hash_new(void (*destroy)(Hash_ent *entry));
extern void Hash_destroy(Hash **hash);
extern void Hash_put(Hash *hash, const char *key, void *value);
extern Hash_ent *Hash_get(Hash *hash, const char *key);
extern void Hash_del(Hash *hash, const char *key);
extern Hash_ent *Hash_entries(Hash *hash);

#endif
