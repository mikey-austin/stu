#ifndef HASH_DEFINED
#define HASH_DEFINED

#define MAX_KEY_LEN 255

/**
 * A struct to contain a hash entry's key and value pair.
 */
typedef struct Hash_ent {
    char  k[MAX_KEY_LEN + 1]; /**< Hash entry key */
    void *v;                  /**< Hash entry value */
} Hash_ent;

/**
 * The main hash table structure.
 */
typedef struct Hash {
    int  size;        /**< The initial hash size. */
    int  num_entries; /**< The number of set elements. */
    void (*destroy)(struct Hash_ent *entry);
    struct Hash_ent  *entries;
} Hash;

extern Hash *Hash_new(int size, void (*destroy)(Hash_ent *entry));
extern void Hash_destroy(Hash **hash);
extern void Hash_reset(Hash *hash);
extern void Hash_put(Hash *hash, const char *key, void *value);
extern void *Hash_get(Hash *hash, const char *key);
extern void Hash_del(Hash *hash, const char *key);
extern int Hash_entries(Hash *hash, Hash_ent ***entries);
extern int Hash_exists(Hash *hash, const char *key);

#endif
