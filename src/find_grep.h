#ifndef FIND_GREP_DEF_H
#define FIND_GREP_DEF_H

#include "macros.h"
#include <stddef.h>

void find_fgrep(const char *RESTRICT needle, const size_t nlen, const char *RESTRICT dir, const size_t dlen);
NOINLINE void fgrep_noinline(const char *RESTRICT needle, const char *RESTRICT filename, const size_t nlen, const size_t flen);

#endif /* FIND_GREP_DEF_H */
