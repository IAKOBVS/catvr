/* musl as a whole is licensed under the following standard MIT license:

----------------------------------------------------------------------
Copyright © 2005-2020 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
----------------------------------------------------------------------

Authors/contributors include:

A. Wilcox
Ada Worcester
Alex Dowad
Alex Suykov
Alexander Monakov
Andre McCurdy
Andrew Kelley
Anthony G. Basile
Aric Belsito
Arvid Picciani
Bartosz Brachaczek
Benjamin Peterson
Bobby Bingham
Boris Brezillon
Brent Cook
Chris Spiegel
Clément Vasseur
Daniel Micay
Daniel Sabogal
Daurnimator
David Carlier
David Edelsohn
Denys Vlasenko
Dmitry Ivanov
Dmitry V. Levin
Drew DeVault
Emil Renner Berthing
Fangrui Song
Felix Fietkau
Felix Janda
Gianluca Anzolin
Hauke Mehrtens
He X
Hiltjo Posthuma
Isaac Dunham
Jaydeep Patil
Jens Gustedt
Jeremy Huntwork
Jo-Philipp Wich
Joakim Sindholt
John Spencer
Julien Ramseier
Justin Cormack
Kaarle Ritvanen
Khem Raj
Kylie McClain
Leah Neukirchen
Luca Barbato
Luka Perkov
M Farkas-Dyck (Strake)
Mahesh Bodapati
Markus Wichmann
Masanori Ogino
Michael Clark
Michael Forney
Mikhail Kremnyov
Natanael Copa
Nicholas J. Kain
orc
Pascal Cuoq
Patrick Oppenlander
Petr Hosek
Petr Skocik
Pierre Carrier
Reini Urban
Rich Felker
Richard Pennington
Ryan Fairfax
Samuel Holland
Segev Finer
Shiz
sin
Solar Designer
Stefan Kristiansson
Stefan O'Rear
Szabolcs Nagy
Timo Teräs
Trutz Behn
Valentin Ochs
Will Dietz
William Haddon
William Pitcock  */

#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include "../lib/librgrep.h"
#include <stdint.h>
/* #include <string.h> */
/* #include <stddef.h> */

#ifndef HAS_MEMMEM
#	ifndef memmem
#		define memmem(haystack, haystacklen, needle, needlelen) strstr(haystack, needle)
#	endif /* memmem */
#endif /* !HAS_MEMMEM */

static char *twobyte_memmem(const unsigned char *h, size_t k, const unsigned char *n)
{
	uint16_t nw = n[0] << 8 | n[1], hw = h[0] << 8 | h[1];
	for (h += 2, k -= 2; k; k--, hw = hw << 8 | *h++)
		if (hw == nw)
			return (char *)h - 2;
	return hw == nw ? (char *)h - 2 : 0;
}

static char *threebyte_memmem(const unsigned char *h, size_t k, const unsigned char *n)
{
	uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8;
	uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8;
	for (h += 3, k -= 3; k; k--, hw = (hw | *h++) << 8)
		if (hw == nw)
			return (char *)h - 3;
	return hw == nw ? (char *)h - 3 : 0;
}

static char *fourbyte_memmem(const unsigned char *h, size_t k, const unsigned char *n)
{
	uint32_t nw = (uint32_t)n[0] << 24 | n[1] << 16 | n[2] << 8 | n[3];
	uint32_t hw = (uint32_t)h[0] << 24 | h[1] << 16 | h[2] << 8 | h[3];
	for (h += 4, k -= 4; k; k--, hw = hw << 8 | *h++)
		if (hw == nw)
			return (char *)h - 4;
	return hw == nw ? (char *)h - 4 : 0;
}

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

static uint8_t g_mtable[256];

#define hash2(p) (((size_t)(p)[0] - ((size_t)(p)[-1] << 3)) % 256)

static char *base_memmem(const unsigned char *hs, size_t hlen, const unsigned char *ne, size_t nlen)
{
	const unsigned char *const cne = ne;
	const unsigned char *const end = hs + hlen - nlen;
	size_t m1 = nlen - 1;
	size_t shift1 = m1 - g_mtable[hash2(ne + m1)];
	size_t tmp;
	while (hs <= end) {
		do {
			hs += m1;
			tmp = g_mtable[hash2(hs)];
		} while (tmp == 0 && hs <= end);
		hs -= tmp;
		if (tmp < m1)
			continue;
		if (memcmp(hs, cne, m1) == 0)
			return (char *)hs;
		hs += shift1;
	}
	return NULL;
}

static void init_memmem_table(const char *ne)
{
	const size_t m1 = strlen(ne) - 1;
	memset(g_mtable, 0, sizeof(g_mtable));
	for (int i = 1; i < m1 - 1; i++)
		g_mtable[hash2(ne + i)] = i;
	g_mtable[hash2(ne + m1)] = m1;
}

static char *g_memmem(const void *h, size_t hlen, const void *n, size_t nlen)
{
       const unsigned char *hs = (unsigned char *)h;
       const unsigned char *ne = (unsigned char *)n;
	if (hlen < nlen)
		return NULL;
	switch (nlen) {
	case 0: return (char *)h;
	case 1: if (*hs == *ne)
			return (char *)hs;
		return (char *)memchr(hs + 4, *ne, hlen - 4);
	case 2: return twobyte_memmem(hs, hlen, ne);
	case 3: return threebyte_memmem(hs, hlen, ne);
	case 4: return fourbyte_memmem(hs, hlen, ne);
	/* Assumes that needle length will never be over 256; */
	/* otherwise check if needle length is over 256 */
	/* and use the default memmem as fallback */
	default: return base_memmem(hs, hlen, ne, nlen);
	}
}

