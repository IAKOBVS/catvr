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

static char *g_memmem(const void *h, size_t hlen, const void *n, size_t nlen)
{
	/* if (unlikely(!h)) */
	/* 	return (char *)h; */
       const unsigned char *hs = (unsigned char *)h;
       const unsigned char *ne = (unsigned char *)n;
	if (hlen < nlen)
		return NULL;
	switch (nlen) {
	case 0:
		return (char *)h;
	case 1:;
		if (*hs == *ne)
			return (char *)hs;
		return (char *)memchr(hs + 4, *ne, hlen - 4);
	case 2:
		return twobyte_memmem((unsigned char *)h, hlen, (unsigned char *)n);
	case 3:
		return threebyte_memmem((unsigned char *)h, hlen, (unsigned char *)n);
	case 4:
		return fourbyte_memmem((unsigned char *)h, hlen, (unsigned char *)n);
	default:
		return (char *)memmem(h, hlen, n, nlen);
	}
}
