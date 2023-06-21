#ifndef GREP_DEF_H
#define GREP_DEF_H

#include "macros.h"
#include <stddef.h>
void fgrep(const char *RESTRICT needle, const char *RESTRICT filename, const size_t nlen, const size_t flen) NONNULL_ALL;

#endif /* GREP_DEF_H */
