#ifndef MMAP_DEF_H
#define MMAP_DEF_H

#include "macros.h"
#include <stddef.h>

void *mmap_open(const char *RESTRICT filename, size_t *RESTRICT filesz, int *RESTRICT fd);
void mmap_close(void *RESTRICT p, const char *RESTRICT filename, size_t filesz, int fd);

#endif /* MMAP_DEF_H */
