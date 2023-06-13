#ifndef LIBGREP_DEF_H
#define LIBGREP_DEF_H

#include "macros.h"
#include <string.h>

#if USE_ANSI_COLORS
#	define ANSI_RED "\x1b[31m"
#	define ANSI_GREEN "\x1b[32m"
#	define ANSI_YELLOW "\x1b[33m"
#	define ANSI_BLUE "\x1b[34m"
#	define ANSI_MAGENTA "\x1b[35m"
#	define ANSI_CYAN "\x1b[36m"
#	define ANSI_RESET "\x1b[0m"
#else
#	define ANSI_RED ""
#	define ANSI_GREEN ""
#	define ANSI_YELLOW ""
#	define ANSI_BLUE ""
#	define ANSI_MAGENTA "\x1b[35m"
#	define ANSI_CYAN ""
#	define ANSI_RESET ""
#endif /* USE_ANSI_COLORS */

#define UINT_LEN 10

#if MAX_NEEDLE_LEN > 256
#	define g_memmem(hs, hlen, ne, nlen) unlikely(ne > 256) ? memmem(hs, hlen, ne, nlen) : g_memmem(hs, hlen, ne, nlen)
typedef size_t needlelen_t;
#else
typedef unsigned int needlelen_t;
#endif

/* Does not nul terminate */
#define itoa_uint_pos(s, n, base, digits)                                        \
	do {                                                                     \
		STATIC_ASSERT(base > 0, "Using negative base in itoa_uint_pos"); \
		unsigned int n_ = n;                                             \
		char *const end = (s) + UINT_LEN - 1;                            \
		(s) = end;                                                       \
		do                                                               \
			*(s)-- = (n_) % (base) + '0';                            \
		while ((n_) /= 10);                                              \
		digits = end - (s)++;                                            \
	} while (0)

static INLINE void append(char *path, const char *dir, size_t dlen, const char *filename)
{
	memcpy(path, dir, dlen);
	*(path += dlen) = '/';
	memcpy(path + 1, filename, dlen + 1);
}

static INLINE char *appendp(char *path, const char *dir, size_t dlen, const char *filename)
{
#if defined(HAS_STPCPY) && defined(HAS_MEMPCPY)
	*(path = (char *)mempcpy(path, dir, dlen)) = '/';
	return stpcpy(path + 1, filename);
#else
	memcpy(path, dir, dlen);
	*(path += dlen) = '/';
	dlen = strlen(filename);
	return (char *)memcpy(path + 1, filename, dlen + 1) + dlen;
#endif /* HAS_STPCPY */
}

#endif /* LIBGREP_DEF_H */
