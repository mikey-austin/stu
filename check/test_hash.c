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

#include <libstu/hash.h>
#include "test.h"

#define TEST_KEY1 "test key 1 - big"
#define TEST_KEY2 "test key 2 - test data"
#define TEST_KEY3 "test key 3 - medium"
#define TEST_KEY4 "test key 4 - larger still"
#define TEST_KEY5 "test key 5 - small"

#define TEST_VAL1 "test value 1"
#define TEST_VAL2 "test value 2"
#define TEST_VAL3 "test value 3"
#define TEST_VAL4 "test value 4"
#define TEST_VAL5 "test value 5"

void
destroy(Hash_ent *entry, void *arg)
{
    long passed = (long) arg;
    if (passed != 1234)
        errx(1, "argument was not passed to destroy callback");

    /* Free the malloced value. */
    if(entry && entry->v) {
        free(entry->v);
    }
}

int
main(void)
{
    Hash *hash;
    char *s, *s1, *s2, *s3, *s4, *s5, *s6, *key;
    int i;

    /* Test hash creation. */
    hash = Hash_new(destroy, (void *) 1234);

    TEST_START;

    /* Create a hash string keys to string values. */
    TEST_OK((HASH_NUM_ENTRIES(hash) == 0), "Hash number of entries is zero as expected");

    /* Test insertion of keys in different orders. */
    s1 = malloc(strlen(TEST_VAL1) + 1); strcpy(s1, TEST_VAL1);
    s2 = malloc(strlen(TEST_VAL2) + 1); strcpy(s2, TEST_VAL2);
    s3 = malloc(strlen(TEST_VAL3) + 1); strcpy(s3, TEST_VAL3);
    s4 = malloc(strlen(TEST_VAL4) + 1); strcpy(s4, TEST_VAL4);
    s5 = malloc(strlen(TEST_VAL5) + 1); strcpy(s5, TEST_VAL5);
    s6 = malloc(strlen(TEST_VAL5) + 1); strcpy(s6, TEST_VAL5);

    Hash_put(hash, TEST_KEY5, s5);
    Hash_put(hash, TEST_KEY4, s4);
    Hash_put(hash, TEST_KEY3, s3);

    Hash_ent *cur = Hash_entries(hash);
    TEST_OK(HASH_NUM_ENTRIES(hash) == 3, "Hash size ok");
    i = 0;
    while (cur) {
        key = cur->k;
        if(!strcmp(key, TEST_KEY3))
            i++;
        if(!strcmp(key, TEST_KEY4))
            i++;
        if(!strcmp(key, TEST_KEY5))
            i++;
        cur = NEXT_ENTRY(cur);
    }
    TEST_OK(i == 3, "All expected keys were in keys list");

    Hash_put(hash, TEST_KEY2, s2);
    Hash_put(hash, TEST_KEY1, s1);

    TEST_OK((HASH_NUM_ENTRIES(hash) == 5), "Hash size is as expected");

    cur = Hash_get(hash, TEST_KEY1);
    s = cur->v;
    TEST_OK((strcmp(s, TEST_VAL1) == 0), "Hash entry 1 inserted correctly");

    cur = Hash_get(hash, TEST_KEY2);
    s = cur->v;
    TEST_OK((strcmp(s, TEST_VAL2) == 0), "Hash entry 2 inserted correctly");

    cur = Hash_get(hash, TEST_KEY3);
    s = cur->v;
    TEST_OK((strcmp(s, TEST_VAL3) == 0), "Hash entry 3 inserted correctly");

    cur = Hash_get(hash, TEST_KEY4);
    s = cur->v;
    TEST_OK((strcmp(s, TEST_VAL4) == 0), "Hash entry 4 inserted correctly");

    cur = Hash_get(hash, TEST_KEY5);
    s = cur->v;
    TEST_OK((strcmp(s, TEST_VAL5) == 0), "Hash entry 5 inserted correctly");

    i = HASH_NUM_ENTRIES(hash);
    Hash_del(hash, TEST_KEY5);
    cur = Hash_get(hash, TEST_KEY5);
    TEST_OK((cur == NULL), "Hash entry 5 deleted correctly");
    TEST_OK(((i - 1) == HASH_NUM_ENTRIES(hash)), "Hash size is as expected");

    /* Test the ability to overwrite values. */
    i = HASH_NUM_ENTRIES(hash);
    Hash_put(hash, TEST_KEY1, s6);
    TEST_OK((i == HASH_NUM_ENTRIES(hash)), "No new entry was added as expected");
    cur = Hash_get(hash, TEST_KEY1);
    s = cur->v;
    TEST_OK((strcmp(s, TEST_VAL5) == 0), "Hash entry overwritten correctly");

    /* Destroy the hash. */
    Hash_destroy(&hash);

    TEST_FINISH;
}
