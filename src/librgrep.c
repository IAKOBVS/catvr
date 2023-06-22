#include "macros.h"
#include <stdio.h>
#include <string.h>

void append(char *RESTRICT path, const char *RESTRICT dir, size_t dlen, const char *RESTRICT filename);
char *appendp(char *RESTRICT path, const char *RESTRICT dir, size_t dlen, const char *RESTRICT filename);
NOINLINE void fgrep_err(const char *RESTRICT msg, const char *RESTRICT filename);

void append(char *RESTRICT path, const char *RESTRICT dir, size_t dlen, const char *RESTRICT filename)
{
	memcpy(path, dir, dlen);
	*(path += dlen) = '/';
	memcpy(path + 1, filename, dlen + 1);
}

char *appendp(char *RESTRICT path, const char *RESTRICT dir, size_t dlen, const char *RESTRICT filename)
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

NOINLINE void fgrep_err(const char *RESTRICT msg, const char *RESTRICT filename)
{
	perror("");
	fprintf(stderr, "%s:%s\n", msg, filename);
}