#include "macros.h"
#include <stdio.h>

NOINLINE void fgrep_err(const char *RESTRICT msg, const char *RESTRICT filename);
NOINLINE void fgrep_err(const char *RESTRICT msg, const char *RESTRICT filename)
{
	perror("");
	fprintf(stderr, "%s:%s\n", msg, filename);
}
