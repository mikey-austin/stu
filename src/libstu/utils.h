#ifndef UTILS_DEFINED
#define UTILS_DEFINED

#include <pwd.h>
#include <stdlib.h>

extern size_t sstrncat(char *dst, const char *src, size_t dsize);
extern size_t sstrncpy(char *dst, const char *src, size_t dsize);

#endif
