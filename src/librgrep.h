#ifndef LIBGREP_DEF_H
#define LIBGREP_DEF_H

#include "macros.h"
#include <string.h>

#if USE_ANSI_COLORS
#	define ANSI_RED     "\x1b[31m"
#	define ANSI_GREEN   "\x1b[32m"
#	define ANSI_YELLOW  "\x1b[33m"
#	define ANSI_BLUE    "\x1b[34m"
#	define ANSI_MAGENTA "\x1b[35m"
#	define ANSI_CYAN    "\x1b[36m"
#	define ANSI_RESET   "\x1b[0m"
#else
#	define ANSI_RED     ""
#	define ANSI_GREEN   ""
#	define ANSI_YELLOW  ""
#	define ANSI_BLUE    ""
#	define ANSI_MAGENTA ""
#	define ANSI_CYAN    ""
#	define ANSI_RESET   ""
#endif /* USE_ANSI_COLORS */

/* Does not nul terminate */
#define itoa_uint_pos(s, n, base, digits)             \
	do {                                          \
		unsigned int n__ = n;                  \
		char *const end__ = (s) + UINT_LEN - 1; \
		(s) = end__;                            \
		do                                    \
			*(s)-- = (n__) % (base) + '0'; \
		while ((n__) /= 10);                   \
		digits = end__ - (s)++;                 \
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
