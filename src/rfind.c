#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "g_memmem.h"
#include "librgrep.h"
#include "unlocked_io.h"

#undef USE_ANSI_COLORS
#define USE_ANSI_COLORS 0

static char g_ln[MAX_LINE_LEN];

/* skip . , .., .git, .vscode */
#define IF_EXCLUDED_DO(filename, action)         \
	do {                                     \
		if ((filename)[0] == '.')        \
			switch ((filename)[1]) { \
			case '.':                \
			case '\0':               \
				action;          \
			}                        \
	} while (0)

static void find(const char *RESTRICT dir, const size_t dlen, const char *RESTRICT ptn, const size_t ptnlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	char fulpath[MAX_PATH_LEN];
	char *g_found;
	size_t g_lnlen;

#define PRINT_LITERAL(s) \
	fwrite((s), 1, sizeof(s) - 1, stdout)

#define DO_REG                                                                                    \
	do {                                                                                      \
		g_lnlen = appendp(g_ln, dir, dlen, ep->d_name) - g_ln;                            \
		if ((g_found = g_memmem(g_ln, g_lnlen, ptn, ptnlen))) {                           \
			flockfile(stdout);                                                        \
			fwrite(g_ln, 1, g_found - g_ln, stdout);                                  \
			PRINT_LITERAL(ANSI_RED);                                                  \
			fwrite(g_found, 1, ptnlen, stdout);                                       \
			PRINT_LITERAL(ANSI_RESET);                                                \
			fwrite(g_found + ptnlen, 1, g_lnlen - ptnlen - (g_found - g_ln), stdout); \
			putchar('\n');                                                            \
			funlockfile(stdout);                                                      \
		}                                                                                 \
	} while (0)

#define DO_DIR                                                                                 \
	do {                                                                                   \
		IF_EXCLUDED_DO(ep->d_name, goto DO_DIR_BREAK__);                               \
		find(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath, ptn, ptnlen); \
DO_DIR_BREAK__:;                                                                               \
	} while (0)

	for (struct dirent *RESTRICT ep; (ep = readdir(dp)); ) {
#ifdef _DIRENT_HAVE_D_TYPE
		switch (ep->d_type) {
		case DT_REG:
			DO_REG;
			break;
		case DT_DIR:
			DO_DIR;
			break;
		}
#else
	struct stat st;
	if (unlikely(stat(dir, &st)))
		continue;
	if (S_ISREG(st.st_mode))
		DO_REG;
	else if (S_ISDIR(st.st_mode))
		DO_DIR;
#endif /* _DIRENT_HAVE_D_TYPE */
	}
	closedir(dp);
}

static void find_all(const char *RESTRICT dir, const size_t dlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	char fulpath[MAX_PATH_LEN];
#define DO_REG_ALL \
	puts(ep->d_name)

#define DO_DIR_ALL                                                                    \
	do {                                                                          \
		IF_EXCLUDED_DO(ep->d_name, goto DO_DIR_BREAK__);                      \
		find_all(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
DO_DIR_BREAK__:;                                                                      \
	} while (0)

	for (struct dirent *RESTRICT ep; (ep = readdir(dp));) {
#ifdef _DIRENT_HAVE_D_TYPE
		switch (ep->d_type) {
		case DT_REG:
			DO_REG_ALL;
			break;
		case DT_DIR:
			DO_DIR_ALL;
			break;
		}
#else
		struct stat st;
		if (unlikely(stat(dir, &st)))
			continue;
		if (S_ISREG(st.st_mode))
			DO_REG;
		else if (S_ISDIR(st.st_mode))
			DO_DIR;
#endif /* _DIRENT_HAVE_D_TYPE */
	}
	closedir(dp);
}

static size_t init_ptn(char *RESTRICT dst, const char *RESTRICT src)
{
	const char *const d = dst;
	for (;; ++src, ++dst) {
		switch (*src) {
			CASE_UPPER
			*dst = *src - 'A' + 'a';
			continue;
		default:
			*dst = *src;
			continue;
		case '\0':
			*dst = '\0';
		}
		break;
	}
	return dst - d;
}

#define IF_FIND_ALL                       \
	do {                              \
		if (argv[1][0] == '\0') { \
			find_all(".", 1); \
			return 1;         \
		}                         \
	} while (0)

int main(int argc, char **argv)
{
	if (argc == 1 || !argv[1][0]) {
		find_all(".", 1);
		return 0;
	}
	char ptnbuf[MAX_NEEDLE_LEN + 1];
	char *ptn;
	char *dir;
	size_t ptnlen;
	size_t dlen;
	switch (argc) {
	case 2:
		ptnlen = init_ptn(ptnbuf, argv[1]);
		ptn = ptnbuf;
		dir = ".";
		dlen = 1;
		break;
	case 3:
		ptnlen = init_ptn(ptnbuf, argv[1]);
		ptn = ptnbuf;
		dir = argv[2];
		dlen = strlen(dir);
		break;
	default:
		fputs("Usage: ./rfind <pattern> <dir>\ndir is PWD by default\n", stderr);
		return 0;
	}
	init_memmem(ptn, ptnlen);
	find(dir, dlen, ptn, ptnlen);
	return 0;
}
