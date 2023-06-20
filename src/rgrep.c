#define PROG_NAME "rgrep"

#include <sys/stat.h>

#include "find_cat.h"
#include "find_grep.h"
#include "g_memmem.h"
#include "grep.h"

static void stat_fail(const char *entry)
{
	fprintf(stderr, PROG_NAME ": %s: Stat failed\n", entry);
}

static void no_such_file(const char *entry)
{
	fprintf(stderr, PROG_NAME ": %s : No such file or directory\n", entry);
}

#define NEEDLE_ARG argv[1]
#define DIR_ARG	   argv[2]

int main(int argc, char **argv)
{
	if (argc == 1 || !argv[1][0]) {
		find_cat(".", 1);
		return 1;
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
		struct stat st;
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
}
