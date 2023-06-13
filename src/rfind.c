#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "fork.h"
#include "g_memmem.h"
#include "librgrep.h"
#include "unlocked_macros.h"

static char *g_found;
static unsigned int g_fuldirlen;
static pid_t pid = 1;
static unsigned int g_child_tot = 0;

/* skip . , .., .git, .vscode */
#define IF_EXCLUDED_DO(filename, action) \
	if (filename[0] == '.')          \
		switch (filename[1]) {   \
		case '.':                \
		case '\0':               \
			action;          \
		}

static void find(const char *RESTRICT dir, const size_t dlen, const char *ptn, const size_t ptnlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	struct dirent *RESTRICT ep;
	char fulpath[MAX_PATH_LEN];
	while ((ep = readdir(dp))) {
#if DEBUG
		printf("d->name: %s\n", ep->d_name);
#endif /* DEBUG */

#define PRINT_LITERAL(s) \
	fwrite(s, 1, sizeof(s) - 1, stdout)

#define DO_REG                                                                                          \
	do {                                                                                            \
		const size_t fulpathlen = appendp(fulpath, dir, dlen, ep->d_name) - fulpath;            \
		if ((g_found = g_memmem(fulpath, fulpathlen, ptn, ptnlen))) {                           \
			flockfile(stdout);                                                              \
			fwrite(fulpath, 1, g_found - fulpath, stdout);                                  \
			PRINT_LITERAL(ANSI_RED);                                                        \
			fwrite(g_found, 1, ptnlen, stdout);                                             \
			PRINT_LITERAL(ANSI_RESET);                                                      \
			fwrite(g_found + ptnlen, 1, fulpathlen - ptnlen - (g_found - fulpath), stdout); \
			putchar('\n');                                                                  \
			funlockfile(stdout);                                                            \
		}                                                                                       \
	} while (0)

#define DO_DIR                               \
	IF_EXCLUDED_DO(ep->d_name, continue) \
	FORK_AND_WAIT(find(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath, ptn, ptnlen))

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
		if (unlikely(stat(dir, &g_st)))
			return;
		if (S_ISREG(g_st.st_mode))
			DO_REG;
		else if (S_ISDIR(g_st.st_mode))
			DO_DIR;
#endif /* _DIRENT_HAVE_D_TYPE */
#if DEBUG
		printf("entries: %s\n", ep->d_name);
#endif /* DEBUG */
	}
	closedir(dp);
}

static INLINE void init_ptn(char *dst, const char *src)
{
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
}

int main(int argc, char **argv)
{
	char ptnbuf[MAX_NEEDLE_LEN + 1];
	char *ptn;
	char *dir;
	switch (argc) {
	case 1:
		ptn = "";
		goto dotdir;
		break;
	case 2:
		init_ptn(ptnbuf, argv[1]);
		ptn = ptnbuf;
	dotdir:
		dir = ".";
		g_fuldirlen = 1;
		break;
	case 3:
		init_ptn(ptnbuf, argv[1]);
		ptn = ptnbuf;
		dir = argv[2];
		g_fuldirlen = strlen(dir);
		break;
	default:
		fputs("Usage: ./rfind <pattern> <dir>\ndir is PWD by default\n", stderr);
		return 0;
	}
	const size_t ptnlen = strlen(ptn);
	init_memmem(ptn, ptnlen);
	find(dir, g_fuldirlen, ptn, ptnlen);
	return 0;
}
