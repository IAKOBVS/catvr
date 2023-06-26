#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <stdlib.h>

#include "config.h"
#include "find_cat.h"
#include "find_grep.h"
#include "g_memmem.h"
#include "globals.h"

#define PROG_NAME "rgrep"

static NOINLINE void stat_fail(const char *entry);
static NOINLINE void no_such_file(const char *entry);
static INLINE void init_g_buf(void);
static INLINE void close_g_buf(void);

static NOINLINE void stat_fail(const char *entry)
{
	fprintf(stderr, PROG_NAME ": %s: Stat failed\n", entry);
}

static NOINLINE void no_such_file(const char *entry)
{
	fprintf(stderr, PROG_NAME ": %s : No such file or directory\n", entry);
}

static INLINE void init_g_buf(void)
{
	g_buf = malloc(MIN_BUF_SZ);
	if (unlikely(!g_buf)) {
		fputs("Can't malloc main buffer\n", stderr);
		exit(1);
	}
	g_bufsz = MIN_BUF_SZ;
}

static INLINE void close_g_buf(void)
{
	free(g_buf);
}

#define GET_NEEDLE_LEN(n, nlen)                   \
	do {                                      \
		if (n[1] == '\0')                 \
			nlen = 1;                 \
		else if (n[2] == '\0')            \
			nlen = 2;                 \
		else if (n[3] == '\0')            \
			nlen = 3;                 \
		else                              \
			nlen = 3 + strlen(n + 3); \
	} while (0)

#define needle (argv[1])
#define dir    (argv[2])

int main(int argc, char **argv)
{
	init_g_buf();
	setvbuf(stdout, NULL, _IOFBF, BUFSIZ);
	if (argc == 1 || !argv[1][0]) {
		find_cat(".", 1);
		return 0;
	}
	size_t nlen;
	GET_NEEDLE_LEN(needle, nlen);
	init_memmem(needle, nlen);
	if (argc == 2)
		goto GREP_ALL;
	switch (dir[0]) {
	case '.':
		if (unlikely(dir[1] == '\0'))
			goto GREP_ALL;
	/* FALLTHROUGH */
	default:
		if (unlikely(stat(dir, &st))) {
			stat_fail(dir);
			return 1;
		}
		if (S_ISREG(st.st_mode)) {
			fgrep_noinline(needle, dir, nlen, strlen(dir));
		} else if (S_ISDIR(st.st_mode)) {
			find_fgrep(needle, nlen, dir, strlen(dir));
		} else {
			no_such_file(dir);
			return 1;
		}
		break;
	case '\0':
GREP_ALL:;
		find_fgrep(needle, nlen, ".", 1);
		break;
	}
	close_g_buf();
	return 0;
}
