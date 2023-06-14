#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "fork.h"
#include "g_memmem.h"
#include "librgrep.h"
#include "unlocked_macros.h"

#define flockfile(fp)
#define funlockfile(fp)

static char g_ln[MAX_LINE_LEN + 1];
static size_t g_lnlen;
static char *g_found;
static pid_t pid = 1;
static unsigned int g_child_tot = 0;

/* #define g_memmem(h, hlen, n, nlen) memmem(h, hlen, n, nlen) */

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

static void find(const char *RESTRICT dir, const size_t dlen, const char *ptn, const size_t ptnlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	struct dirent *RESTRICT ep;
	char fulpath[MAX_PATH_LEN];
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
		IF_EXCLUDED_DO(ep->d_name, goto CONT);                                         \
		find(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath, ptn, ptnlen); \
	} while (0)

#ifdef _DIRENT_HAVE_D_TYPE
	while ((ep = readdir(dp))) {
#if DEBUG
		printf("d->name: %s\n", ep->d_name);
#endif /* DEBUG */
		switch (ep->d_type) {
		case DT_REG:
			DO_REG;
			break;
		case DT_DIR:
			DO_DIR;
			break;
		}
#else
		if (unlikely(stat(dir, &g_st)))
			continue;
		if (S_ISREG(g_st.st_mode))
			DO_REG;
		else if (S_ISDIR(g_st.st_mode))
			DO_DIR;
#endif /* _DIRENT_HAVE_D_TYPE */
#if DEBUG
		printf("entries: %s\n", ep->d_name);
#endif /* DEBUG */
	CONT:;
	}
	closedir(dp);
}

static size_t init_ptn(char *dst, const char *src)
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

int main(int argc, char **argv)
{
	char ptnbuf[MAX_NEEDLE_LEN + 1];
	char *ptn;
	char *dir;
	size_t ptnlen;
	size_t dlen;
	switch (argc) {
	case 1:
		ptn = "";
		ptnlen = 1;
		goto dotdir;
		break;
	case 2:
		ptnlen = init_ptn(ptnbuf, argv[1]);
		ptn = ptnbuf;
	dotdir:
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
