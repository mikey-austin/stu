#ifndef PARSE_DEFINED
#define PARSE_DEFINED

#include "sv.h"
#include "env.h"

extern Sv *Parse_buf(Env *, const char *);
extern Sv *Parse_file(Env *, const char *);

#endif
