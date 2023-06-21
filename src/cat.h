#ifndef CAT_DEF_H
#define CAT_DEF_H

#include "macros.h"
#include <stddef.h>

void cat(const char *RESTRICT filename, const size_t flen) NONNULL_ALL;

#endif /* CAT_DEF_H */
