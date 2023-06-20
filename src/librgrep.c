#include "macros.h"
#include <stdio.h>
#include <string.h>

void append(char *path, const char *dir, size_t dlen, const char *filename)
{
	memcpy(path, dir, dlen);
	*(path += dlen) = '/';
	memcpy(path + 1, filename, dlen + 1);
}

char *appendp(char *path, const char *dir, size_t dlen, const char *filename)
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

void fgrep_err(const char *RESTRICT msg, const char *RESTRICT filename)
{
	perror("");
	fprintf(stderr, "%s:%s\n", msg, filename);
}