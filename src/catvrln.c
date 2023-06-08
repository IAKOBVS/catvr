#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../lib/libcatvr.h"

#define DEBUG 0

#ifndef UINT_LEN
#	define UINT_LEN 10
#endif /* UINT_LEN */

#define MAX_LINE_LEN 4096
#define MAX_PATH_LEN 4096

#if NO_ANSI
#	 define ANSI_RED ""
#	 define ANSI_GREEN ""
#	 define ANSI_RESET ""
#endif /* NO_ANSI */

char g_ln[MAX_LINE_LEN];
char *g_lnp;
unsigned int g_NL;
char g_NLbuf[UINT_LEN];
char *g_NLbufp;
unsigned int g_NLbufdigits;
size_t g_fuldirlen;
struct stat g_st;
int g_c;

#ifdef HAS_FGETC_UNLOCKED
#	define fgetc(c) fgetc_unlocked(c)
#endif

#ifdef HAS_GETC_UNLOCKED
#	define getc(fp) getc_unlocked(fp)
#endif

#ifdef HAS_PUTCHAR_UNLOCKED
#	define putchar(c) putchar_unlocked(c)
#endif

#ifdef HAS_FGETS_UNLOCKED
#	define fgets(s, N, fp) fgets_unlocked(s, N, fp)
#endif

#ifdef HAS_FWRITE_UNLOCKED
#	define fwrite(s, sz, N, fp) fwrite_unlocked(s, sz, N, fp)
#endif

#define CPY_N_ADV(dst, src)                        \
	do {                                       \
		memcpy(dst, src, sizeof(src) - 1); \
		(dst) += (sizeof(src) - 1);        \
	} while (0)

#define CPY_N_ADV_LEN(dst, src, n)   \
	do {                         \
		memcpy(dst, src, n); \
		(dst) += (n);        \
	} while (0)

static INLINE void catv(const char *RESTRICT filename, const size_t flen)
{
	FILE *RESTRICT fp = fopen(filename, "r");
	if (unlikely(!fp))
		return;
	filename = filename + g_fuldirlen + 1;
	g_lnp = g_ln;
	CPY_N_ADV(g_lnp, ANSI_RED);
	CPY_N_ADV_LEN(g_lnp, filename, flen);
	CPY_N_ADV(g_lnp, ANSI_RESET ":" ANSI_GREEN "1" ANSI_RESET ":");
	/* fwrite(ANSI_RED, 1, sizeof(ANSI_RED) - 1, stdout); */
	/* fwrite(filename, 1, flen, stdout); */
	/* fwrite(ANSI_RESET ":" ANSI_GREEN "1" ANSI_RESET ":", 1, sizeof(ANSI_RESET ":" ANSI_GREEN "1" ANSI_RESET ":") - 1, stdout); */
	g_NL = 2;
	for (;; ++g_lnp) {
		switch (*g_lnp = getc(fp)) {
		default:
		case '\t':
			/* putchar(g_c); */
			continue;
		case '\n':
			fwrite(g_ln, 1, g_lnp - g_ln + 1, stdout);
			g_lnp = g_ln;
			CPY_N_ADV(g_lnp, ANSI_RED);
			CPY_N_ADV_LEN(g_lnp, filename, flen);
			CPY_N_ADV(g_lnp, ANSI_RESET ":" ANSI_GREEN);
			g_NLbufp = g_NLbuf;
			itoa_uint_pos(g_NLbufp, g_NL, 10, g_NLbufdigits);
			CPY_N_ADV_LEN(g_lnp, g_NLbufp, g_NLbufdigits);
			CPY_N_ADV(g_lnp, ANSI_RESET ":");
			/* fwrite("\n" ANSI_RED, 1, sizeof("\n" ANSI_RED) - 1, stdout); */
			/* fwrite(filename, 1, flen, stdout); */
			/* fwrite(ANSI_RESET ":" ANSI_GREEN, 1, sizeof(ANSI_RESET ":" ANSI_GREEN) - 1, stdout); */
			/* fwrite(g_NLbufp, 1, g_NLbufdigits, stdout); */
			/* fwrite(ANSI_RESET ":", 1, sizeof(ANSI_RESET ":") - 1, stdout); */
			--g_lnp;
			++g_NL;
			continue;
		case EOF:
			*g_lnp = '\n';
			fwrite(g_ln, 1, g_lnp - g_ln + 1, stdout);
			/* putchar('\n'); */
		case '\0':
		CASE_UNPRINTABLE_WO_NUL_TAB_NL
			;
		}
		break;
	}
	fclose(fp);
}

/* skip . , .., .git, .vscode */
#define IF_EXCLUDED_DO(filename, action)       \
	if (filename[0] == '.')                \
		switch (filename[1]) {         \
		case '.':                      \
		case '\0':                     \
			action;                \
			break;                 \
		case 'g':                      \
			if (filename[2] == 'i' \
			&& filename[3] == 't') \
				action;        \
			break;                 \
		case 'v':                      \
			if (filename[2] == 's' \
			&& filename[3] == 'c'  \
			&& filename[4] == 'o'  \
			&& filename[5] == 'd'  \
			&& filename[6] == 'e') \
				action;        \
			break;                 \
		}

static void findall(const char *RESTRICT dir, const size_t dlen)
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
#ifdef _DIRENT_HAVE_D_TYPE
		switch (ep->d_type) {
		case DT_REG:
			catv(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - (fulpath + g_fuldirlen) - 1);
			break;
		case DT_DIR:
			/* skip . , .., .git, .vscode */
			IF_EXCLUDED_DO(ep->d_name, continue)
			findall(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath);
			break;
		}
#else
		if (unlikely(stat(dir, &g_st)))
			return;
		if (S_ISREG(g_st.st_mode)) {
			catv(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - (fulpath + g_fuldirlen) - 1);
		} else if (S_ISDIR(g_st.st_mode)) {
			IF_EXCLUDED_DO(ep->d_name, continue)
			findall(fulpath, dlen = appendp(fulpath, dir, dlen, ep->d_name) - fulpath);
		}
#endif /* _DIRENT_HAVE_D_TYPE */
#if DEBUG
		printf("entries: %s\n", ep->d_name);
#endif /* DEBUG */
	}
	closedir(dp);
}

#define DIRECTORY argv[1]

int main(int argc, char **argv)
{
	if (unlikely(argc == 1))
		goto GET_CWD;
	switch (DIRECTORY[0]) {
	case '.':
		if (unlikely(DIRECTORY[1] == '\0'))
			goto GET_CWD;
		/* FALLTHROUGH */
	default:
		if (unlikely(stat(DIRECTORY, &g_st))) {
			printf("%s not a valid file or dir\n", DIRECTORY);
			return EXIT_FAILURE;
		}
		if (unlikely(S_ISREG(g_st.st_mode))) {
			argv[2] = strrchr(DIRECTORY, '/');
			if ((argv[2] = strrchr(DIRECTORY, '/'))) {
				g_fuldirlen = argv[2] - DIRECTORY;
				catv(DIRECTORY, strlen(DIRECTORY + g_fuldirlen) - 1);
			} else {
				char cwd[MAX_PATH_LEN];
				getcwd(cwd, MAX_PATH_LEN);
				const size_t clen = strlen(cwd);
				cwd[clen] = '/';
				g_fuldirlen = clen;
#ifdef HAS_STPCPY
				catv(cwd, stpcpy(cwd + clen + 1, DIRECTORY) - cwd - clen - 1);
#else
				const size_t dlen = strlen(DIRECTORY);
				memcpy(cwd + clen + 1, DIRECTORY, dlen + 1);
				catv(cwd, dlen);
#endif /* HAS_STPCPY */
			}
		} else {
			g_fuldirlen = strlen(DIRECTORY);
			findall(DIRECTORY, g_fuldirlen);
		}
		break;
	case '\0':
	GET_CWD:;
		char cwd[MAX_PATH_LEN];
		getcwd(cwd, MAX_PATH_LEN);
		g_fuldirlen = strlen(cwd);
		findall(cwd, g_fuldirlen);
		break;
	}
	return EXIT_SUCCESS;
}
