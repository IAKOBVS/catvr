#ifndef G_MEMMEM_DEF_H
#define G_MEMMEM_DEF_H

#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include "librgrep.h"
#include <stdint.h>
#include <string.h>

#ifndef HAS_MEMMEM
#	ifndef memmem
#		define memmem(haystack, haystacklen, needle, needlelen) strstr(haystack, needle)
#	endif /* memmem */
#endif /* !HAS_MEMMEM */

extern uint8_t g_mtable[256];
void init_memmem(const char *RESTRICT ne, const size_t nlen);
static INLINE void *g_memmem(const void *RESTRICT h, size_t hlen, const void *RESTRICT n, size_t nlen);

#define hash2(p) (((size_t)(p)[0] - ((size_t)(p)[-1] << 3)) % 256)

/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* modified to use a global table instead of a local one */
static INLINE void *g_memmem(const void *RESTRICT h, size_t hlen, const void *RESTRICT n, size_t nlen)
{
	if (hlen < nlen)
		return NULL;
	const unsigned char *hs = (unsigned char *)h;
	const unsigned char *ne = (unsigned char *)n;
	const unsigned char *const end = hs + hlen - nlen;
	switch (nlen) {
	case 0:
		return (char *)h;
	case 1:
		if (*hs == *ne)
			return (void *)hs;
		return (void *)memchr(hs, *ne, hlen);
	case 2: {
		uint32_t nw = ne[0] << 16 | ne[1], hw = hs[0] << 16 | hs[1];
		for (hs++; hs <= end && hw != nw;)
			hw = hw << 16 | *++hs;
		return hw == nw ? (char *)hs - 1 : NULL;
	}
	}
	/* Assumes that needle length will never be over 256; */
	/* otherwise check if needle length is over 256 */
	/* and use the default memmem as fallback */
	/* if (unlikely(nlen > 256)) */
	/* 	memmem(h, hlen, n, nlen); */
	size_t m1 = nlen - 1;
	size_t shift1 = m1 - g_mtable[hash2(ne + m1)];
	const unsigned char hash = hash2(ne + m1);
	g_mtable[hash] = m1;
	size_t tmp;
	while (hs <= end) {
		do {
			hs += m1;
			tmp = g_mtable[hash2(hs)];
		} while (tmp == 0 && hs <= end);
		hs -= tmp;
		if (tmp < m1)
			continue;
#define reset_table \
	g_mtable[hash] = 0
		if (!memcmp(hs, n, m1)) {
			reset_table;
			return (void *)hs;
		}
		hs += shift1;
	}
	reset_table;
	return NULL;
}

#undef hash2
#undef reset_table

#endif /* G_MEMMEM_DEF_H */
