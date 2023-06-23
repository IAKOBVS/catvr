#ifndef G_MEMMEM_DEF_H
#define G_MEMMEM_DEF_H

#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include "librgrep.h"
#include <stdint.h>

#ifndef HAS_MEMMEM
#	ifndef memmem
#		define memmem(haystack, haystacklen, needle, needlelen) strstr(haystack, needle)
#	endif /* memmem */
#endif /* !HAS_MEMMEM */

uint8_t g_mtable[256];
void init_memmem(const char *RESTRICT ne, const size_t nlen);

#define G_MEMMEM_hash2__(p) (((size_t)(p)[0] - ((size_t)(p)[-1] << 3)) % 256)

void init_memmem(const char *RESTRICT ne, const size_t nlen)
{
	memset(g_mtable, 0, sizeof(g_mtable));
	for (unsigned int i = 1; i < nlen - 1; i++)
		g_mtable[G_MEMMEM_hash2__(ne + i)] = i;
}

#undef G_MEMMEM_hash2__

#endif /* G_MEMMEM_DEF_H */
