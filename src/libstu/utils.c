#include <string.h>

#include "utils.h"

extern size_t
sstrncat(char *dst, const char *src, size_t dsize)
{
    const size_t slen = strlen(src);
    const size_t dlen = strnlen(dst, dsize);

    if(dlen != dsize) {
        size_t count = slen;
        if(count > dsize - dlen - 1)
            count = dsize - dlen - 1;
        dst += dlen;
        memcpy(dst, src, count);
        dst[count] = '\0';
    }

    return slen + dlen;
}

extern size_t
sstrncpy(char *dst, const char *src, size_t dsize)
{
    const size_t slen = strlen(src);

    if(dsize != 0) {
        const size_t dlen = dsize > slen ? slen : dsize - 1;
        memcpy(dst, src, dlen);
        dst[dlen] = '\0';
    }

    return slen;
}
