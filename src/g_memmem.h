#ifndef G_MEMMEM_DEF_H
#define G_MEMMEM_DEF_H

#include "librgrep.h"
#include <stdint.h>

void init_memmem(const char *RESTRICT ne, const int nlen);
void *g_memmem(const void *RESTRICT h, size_t hlen, const void *RESTRICT n, size_t nlen);

#endif /* G_MEMMEM_DEF_H */