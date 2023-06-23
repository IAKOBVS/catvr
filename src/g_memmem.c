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
