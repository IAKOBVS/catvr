#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../lib/libcatvr.h"

enum {
	MAX_LINE_LEN = 4096,
	MAX_PATH_LEN = 4096,
};
char g_ln[MAX_LINE_LEN];
char *g_lnp;
int g_lnlen;
int g_NL = 0;
int g_fuldirlen;
struct stat g_st;

#ifdef HAS_FGETS_UNLOCKED
#	define fgets(s, N, fp) fgets_unlocked(s, N, fp)
#endif

#ifndef HAS_MEMMEM
#	define memmem(haystack, haystacklen, needle, needlelen) strstr(haystack, needle)
#endif /* !HAS_MEMMEM */

#ifdef HAS_MEMMEM
static INLINE int fgrep(const char *ptn, const size_t ptnlen, const char *filename)
#else
static INLINE int fgrep(const char *ptn, const char *filename)
#endif /* HAS_MEMMEM */
{
	FILE *fp = fopen(filename, "r");
	if (unlikely(!fp))
		return 0;
	g_NL = 0;
	while ((g_lnp = fgets(g_ln, MAX_LINE_LEN, fp))) {
		for (;; ++g_lnp) {
			switch (*g_lnp) {
			case '\0':
			CASE_UNPRINTABLE_WO_NUL_TAB_NL
				goto OUT;
			default:
			case '\t':
				continue;
			case '\n':
			case EOF:
#ifdef HAS_MEMMEM
				g_lnlen = g_lnp - g_ln;
#endif /* HAS_MEMMEM */
			}
			break;
		}
		++g_NL;
		if ((memmem(g_ln, g_lnlen, ptn, ptnlen)))
			printf(ANSI_RED "%s" ANSI_RESET ":" ANSI_GREEN "%d" ANSI_RESET ":%s", filename + g_fuldirlen + 1, g_NL, g_ln);
	}
OUT:
	fclose(fp);
	return 1;
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

#ifdef _DIRENT_HAVE_D_TYPE

#define DO_IF_REG(FUNC_SELF, FUNC_REG, ...)                                                                 \
	switch (ep->d_type) {                                                                               \
		case DT_REG:                                                                                \
			append(fulpath, dir, dlen, ep->d_name);                                             \
			FUNC_REG(__VA_ARGS__);                                                              \
			break;                                                                              \
		case DT_DIR:                                                                                \
			/* skip . , .., .git, .vscode */                                                    \
			IF_EXCLUDED_DO(ep->d_name, continue)                                                \
			FUNC_SELF(ptn, ptnlen, fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
	}                                                                                                   \

#else

#define DO_IF_REG(FUNC_SELF, FUNC_REG, ...)                                                         \
	if (unlikely(stat(dir, &g_st)))                                                             \
		continue;                                                                           \
	if (S_ISREG(g_st.st_mode)) {                                                                \
		append(fulpath, dir, dlen, ep->d_name);                                             \
		FUNC_REG(__VA_ARGS__);                                                              \
	} else if (S_ISDIR(g_st.st_mode)) {                                                         \
		IF_EXCLUDED_DO(ep->d_name, continue)                                                \
		FUNC_SELF(ptn, ptnlen, fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
	}

#endif /* _DIRENT_HAVE_D_TYPE */

#define DEF_FIND_T(F, DO, ...)                                                         \
static int F(const char *ptn, const size_t ptnlen, const char *dir, const size_t dlen) \
{                                                                                      \
	DIR *dp = opendir(dir);                                                        \
	if (unlikely(!dp))                                                             \
		return 0;                                                              \
	struct dirent *ep;                                                             \
	char fulpath[MAX_PATH_LEN];                                                    \
	while ((ep = readdir(dp))) {                                                   \
		DO_IF_REG(F, DO, __VA_ARGS__);                                         \
	}                                                                              \
	closedir(dp);                                                                  \
	return 1;                                                                      \
}

static void usage(void)
{
	fputs("Usage: ./fgrepr <ptn> <dir or file> <flag>\nif <dir or file> is not provided, it defaults to $PWD\nif <flag> is -i, it will match case insensitively; otherwise, it will match literally\n", stderr);
	exit(1);
}

DEF_FIND_T(find_fgrep, fgrep,
		ptn, ptnlen, fulpath)

int main(int argc, char **argv)
{
	if (unlikely(argc == 1))
		usage();
	if (unlikely(!argv[1][0]))
		usage();
	char pattern[MAX_LINE_LEN];
	char *pp = pattern;
	char *g_nlp = argv[1];
	for (;; ++g_nlp) {
		switch (*g_nlp) {
		CASE_UPPER
			*pp = *g_nlp - 'A' + 'a';
			continue;
		default:
			*pp = *g_nlp;
			continue;
		case '\0':;
		}
		break;
	}
	*pp = '\0';
	if (argc == 2)
		goto GET_CWD;
	switch (argv[2][0]) {
	case '.':
		if (unlikely(argv[2][1] == '\0'))
			goto GET_CWD;
		/* FALLTHROUGH */
	default:
		if (unlikely(stat(argv[2], &g_st))) {
			printf("%s not a valid file or dir\n", argv[2]);
			return 1;
		}
		if (unlikely(S_ISREG(g_st.st_mode))) {
			g_fuldirlen = strrchr(argv[2], '/') - argv[2];
			fgrep(pattern, strlen(pattern), argv[2]);
		} else {
			g_fuldirlen = strlen(argv[2]);
			find_fgrep(pattern, strlen(pattern), argv[2], g_fuldirlen);
		}
		break;
	case '\0':
	GET_CWD:;
		char cwd[MAX_PATH_LEN];
		getcwd(cwd, MAX_PATH_LEN);
		g_fuldirlen = strlen(cwd);
		find_fgrep(pattern, strlen(pattern), cwd, g_fuldirlen);
		break;
	}
	return 0;
}
