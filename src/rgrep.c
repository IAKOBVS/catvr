#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#define PROG_NAME "rgrep"

#include <stdlib.h>
#include <sys/stat.h>

#include "config.h"
#include "find_cat.h"
#include "find_grep.h"
#include "g_memmem.h"
#include "globals.h"
#include "grep.h"

void stat_fail(const char *entry);
void no_such_file(const char *entry);
void init_g_buf(void);
void close_g_buf(void);

NOINLINE void stat_fail(const char *entry)
{
	fprintf(stderr, PROG_NAME ": %s: Stat failed\n", entry);
}

NOINLINE void no_such_file(const char *entry)
{
	fprintf(stderr, PROG_NAME ": %s : No such file or directory\n", entry);
}

INLINE void init_g_buf(void)
{
	g_buf = malloc(MIN_BUF_SZ);
	if (unlikely(!g_buf)) {
		fputs("Can't allocate memory", stderr);
		exit(1);
	}
	g_bufsz = MIN_BUF_SZ;
}

INLINE void close_g_buf(void)
{
	free(g_buf);
}

#define NEEDLE_ARG argv[1]
#define DIR_ARG	   argv[2]

int main(int argc, char **argv)
{
	init_g_buf();
	if (argc == 1 || !argv[1][0]) {
		find_cat(".", 1);
		return 0;
	}
	const size_t nlen = strlen(NEEDLE_ARG);
	init_memmem(NEEDLE_ARG, nlen);
	if (argc == 2)
		goto GREP_ALL;
	switch (DIR_ARG[0]) {
	case '.':
		if (unlikely(DIR_ARG[1] == '\0'))
			goto GREP_ALL;
	/* FALLTHROUGH */
	default: {
		if (unlikely(stat(DIR_ARG, &st))) {
			stat_fail(DIR_ARG);
			return 1;
		}
		if (unlikely(S_ISREG(st.st_mode))) {
			fgrep(NEEDLE_ARG, DIR_ARG, nlen, strlen(DIR_ARG));
		} else if (S_ISDIR(st.st_mode)) {
			find_fgrep(NEEDLE_ARG, nlen, DIR_ARG, strlen(DIR_ARG));
		} else {
			no_such_file(DIR_ARG);
			return 1;
		}
	} break;
	case '\0':
GREP_ALL:;
		find_fgrep(NEEDLE_ARG, nlen, ".", 1);
		break;
	}
	close_g_buf();
}
