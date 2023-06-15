#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#define PROG_NAME "rgrep"

#include <dirent.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "fork.h"
#include "g_memmem.h"
#include "globals.h"
#include "librgrep.h"
#include "unlocked_macros.h"

static INLINE void fgrep(const char *needle, const char *filename, const size_t needlelen, const size_t flen)
{
	FILE *fp = fopen(filename, "r");
	if (unlikely(!fp))
		return;
	g_first_match = 0;
	g_NL = 1;
	g_lnp = g_ln;
	g_lnlowerp = g_lnlower;
	if (likely(g_fuldirlen))
		filename = filename + g_fuldirlen + 1;

#define PRINT_LITERAL(s) \
	fwrite(s, 1, sizeof(s) - 1, stdout)

	do {
	CONT:
		g_c = getc(fp);
		switch (g_table[g_c + 1]) {
		case WANTED_UPPER:
			if (!g_first_match)
				g_first_match = g_lnp - g_ln;
			/* FALLTHROUGH */
		case UPPER:
			*g_lnp = g_c;
			*g_lnlowerp = g_c - 'A' + 'a';
			break;
		case WANTED:
			if (!g_first_match)
				g_first_match = g_lnp - g_ln;
			/* FALLTHROUGH */
		default:
			*g_lnp = g_c;
			*g_lnlowerp = g_c;
			break;
		case NEWLINE:
			if (g_first_match) {
#define PRINT_LN                                                                                                   \
	do {                                                                                                       \
		g_lnlen = g_lnp - g_ln;                                                                            \
		if ((g_found = g_memmem(g_lnlower + g_first_match, g_lnlen - g_first_match, needle, needlelen))) { \
			g_found = g_ln + (g_found - g_lnlower);                                                    \
			g_NLbufp = g_NLbuf;                                                                        \
			itoa_uint_pos(g_NLbufp, g_NL, 10, g_NLbufdigits);                                          \
			flockfile(stdout);                                                                         \
			PRINT_LITERAL(ANSI_RED);                                                                   \
			fwrite(filename, 1, flen, stdout);                                                         \
			PRINT_LITERAL(ANSI_RESET ":" ANSI_GREEN);                                                  \
			fwrite(g_NLbufp, 1, g_NLbufdigits, stdout);                                                \
			PRINT_LITERAL(ANSI_RESET ":");                                                             \
			fwrite(g_ln, 1, g_found - g_ln, stdout);                                                   \
			PRINT_LITERAL(ANSI_RED);                                                                   \
			fwrite(g_found, 1, needlelen, stdout);                                                     \
			PRINT_LITERAL(ANSI_RESET);                                                                 \
			fwrite(g_found + needlelen, 1, g_lnlen - (g_found - g_ln + needlelen), stdout);            \
			putchar('\n');                                                                             \
			fflush_unlocked(stdout);                                                                   \
			funlockfile(stdout);                                                                       \
		}                                                                                                  \
	} while (0)
				PRINT_LN;
				g_first_match = 0;
			}
			++g_NL;
			g_lnp = g_ln;
			g_lnlowerp = g_lnlower;
			goto CONT;
		case END_OF_FILE:
			if (g_first_match)
				PRINT_LN;
			/* FALLTHROUGH */
		case REJECT:
			goto OUT;
		}
		++g_lnlowerp, ++g_lnp;
	} while (g_lnp - g_ln != MAX_LINE_LEN);
OUT:
	fclose(fp);
}

#define IF_EXCLUDED_REG_DO(filename, action) \
	do {                                 \
		if ((filename)[0] == '.'     \
		&& (filename)[1] == 'c'      \
		&& (filename)[2] == 'l'      \
		&& (filename)[3] == 'a'      \
		&& (filename)[4] == 'n'      \
		&& (filename)[5] == 'g'      \
		&& (filename)[6] == '-'      \
		&& (filename)[7] == 'f'      \
		&& (filename)[8] == 'o'      \
		&& (filename)[9] == 'r'      \
		&& (filename)[10] == 'm'     \
		&& (filename)[11] == 'a'     \
		&& (filename)[12] == 't')    \
			action;              \
	} while (0)

/* skip . , .., .git, .vscode */
#define IF_EXCLUDED_DIR_DO(filename, action)             \
	do {                                             \
		if ((filename)[0] == '.')                \
			switch ((filename)[1]) {         \
			case '.':                        \
			case '\0':                       \
				action;                  \
				break;                   \
			case 'g':                        \
				if ((filename)[2] == 'i' \
				&& (filename)[3] == 't') \
					action;          \
				break;                   \
			case 'v':                        \
				if ((filename)[2] == 's' \
				&& (filename)[3] == 'c'  \
				&& (filename)[4] == 'o'  \
				&& (filename)[5] == 'd'  \
				&& (filename)[6] == 'e') \
					action;          \
				break;                   \
			}                                \
	} while (0)

#define FIND_FGREP_DO_REG(FUNC_REG, USE_LEN)                                                                                         \
	do {                                                                                                                         \
		IF_EXCLUDED_REG_DO(ep->d_name, goto CONT);                                                                           \
		if (USE_LEN)                                                                                                         \
			FUNC_REG(needle, fulpath, needlelen, appendp(fulpath, dir, dlen, ep->d_name) - (fulpath + g_fuldirlen) - 1); \
		else                                                                                                                 \
			FUNC_REG(needle, fulpath, 0, 0);                                                                             \
	} while (0)

#define FIND_FGREP_DO_DIR(FUNC_SELF)                                                                                     \
	do {                                                                                                             \
		IF_EXCLUDED_DIR_DO(ep->d_name, goto CONT);                                                               \
		FORK_AND_WAIT(FUNC_SELF(needle, needlelen, fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath)); \
	} while (0)

#ifdef _DIRENT_HAVE_D_TYPE

#	define IF_DIR_RECUR_IF_REG_DO(FUNC_SELF, FUNC_REG, USE_LEN) \
		switch (ep->d_type) {                                \
		case DT_REG:                                         \
			FIND_FGREP_DO_REG(FUNC_REG, USE_LEN);        \
			break;                                       \
		case DT_DIR:                                         \
			FIND_FGREP_DO_DIR(FUNC_SELF);                \
		}

#else

#	define IF_DIR_RECUR_IF_REG_DO(FUNC_SELF, FUNC_REG, USE_LEN) \
		if (unlikely(stat(dir, &g_st)))                      \
			continue;                                    \
		if (S_ISREG(g_st.st_mode))                           \
			FIND_FGREP_DO_REG(FUNC_REG, USE_LEN);        \
		else if (S_ISDIR(g_st.st_mode))                      \
			FIND_FGREP_DO_DIR(FUNC_SELF);

#endif /* _DIRENT_HAVE_D_TYPE */

#define DEF_FIND_T(F, DO, USE_LEN)                                                                    \
	static void F(const char *needle, const size_t needlelen, const char *dir, const size_t dlen) \
	{                                                                                             \
		DIR *dp = opendir(dir);                                                               \
		if (unlikely(!dp))                                                                    \
			return;                                                                       \
		struct dirent *ep;                                                                    \
		char fulpath[MAX_PATH_LEN];                                                           \
		while ((ep = readdir(dp))) {                                                          \
			IF_DIR_RECUR_IF_REG_DO(F, DO, USE_LEN)                                        \
		CONT:;                                                                                \
		}                                                                                     \
		closedir(dp);                                                                         \
		return;                                                                               \
	}

DEF_FIND_T(find_fgrep, fgrep, 1)

static INLINE void cat(const char *RESTRICT filename, const size_t flen)
{
	FILE *RESTRICT fp = fopen(filename, "r");
	if (unlikely(!fp))
		return;
	if (likely(g_fuldirlen))
		filename = filename + g_fuldirlen + 1;
	g_lnp = g_ln;
	g_NL = 1;
	do {
	CONT:;
		switch (*g_lnp = getc(fp)) {
		default:
		case '\t':
			break;
		case '\n':
			if (unlikely(g_lnp - g_ln == 0))
				goto CONT;
#define CAT_PRINT_LN                                              \
	do {                                                      \
		g_NLbufp = g_NLbuf;                               \
		itoa_uint_pos(g_NLbufp, g_NL, 10, g_NLbufdigits); \
		flockfile(stdout);                                \
		PRINT_LITERAL(ANSI_RED);                          \
		fwrite(filename, 1, flen, stdout);                \
		PRINT_LITERAL(ANSI_RESET ":" ANSI_GREEN);         \
		fwrite(g_NLbufp, 1, g_NLbufdigits, stdout);       \
		PRINT_LITERAL(ANSI_RESET ":");                    \
		fwrite(g_ln, 1, g_lnp - g_ln + 1, stdout);        \
		fflush_unlocked(stdout);                          \
		funlockfile(stdout);                              \
	} while (0)
			CAT_PRINT_LN;
			++g_NL;
			g_lnp = g_ln;
			goto CONT;
		case EOF:
			if (unlikely(g_lnp - g_ln == 0))
				break;
			*g_lnp = '\n';
			CAT_PRINT_LN;
		case '\0':
			CASE_UNPRINTABLE_WO_NUL_TAB_NL
			goto OUT;
		}
		++g_lnp;
	} while (g_lnp - g_ln != MAX_LINE_LEN);
OUT:
	fclose(fp);
}

static void find_cat(const char *RESTRICT dir, const size_t dlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	struct dirent *RESTRICT ep;
	char fulpath[MAX_PATH_LEN];

#define FIND_CAT_DO_REG                                                                              \
	do {                                                                                         \
		IF_EXCLUDED_REG_DO(ep->d_name, goto CONT);                                           \
		cat(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - (fulpath + g_fuldirlen) - 1); \
	} while (0)

#define FIND_CAT_DO_DIR                                                                              \
	do {                                                                                         \
		IF_EXCLUDED_DIR_DO(ep->d_name, goto CONT);                                           \
		FORK_AND_WAIT(find_cat(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath)); \
	} while (0)

	while ((ep = readdir(dp))) {
#if DEBUG
		printf("d->name: %s\n", ep->d_name);
#endif /* DEBUG */
#ifdef _DIRENT_HAVE_D_TYPE
		switch (ep->d_type) {
		case DT_REG:
			FIND_CAT_DO_REG;
			break;
		case DT_DIR:
			FIND_CAT_DO_DIR;
			break;
		}
#else
		if (unlikely(stat(dir, &g_st)))
			continue;
		if (S_ISREG(g_st.st_mode))
			FIND_CAT_DO_REG;
		else if (S_ISDIR(g_st.st_mode))
			FIND_CAT_DO_DIR;
#endif /* _DIRENT_HAVE_D_TYPE */
#if DEBUG
		printf("entries: %s\n", ep->d_name);
#endif /* DEBUG */
	CONT:;
	}
	closedir(dp);
}

static size_t init_needle(char *dst, const char *src)
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

static void init_fgrep(const char needle)
{
	g_table[(unsigned char)needle + 1] = WANTED;
	g_table[(unsigned char)needle - 'a' + 'A' + 1] = WANTED_UPPER;
}

static void stat_fail(const char *entry)
{
	fprintf(stderr, PROG_NAME ": %s: Stat failed\n", entry);
}

static void no_such_file(const char *entry)
{
	fprintf(stderr, PROG_NAME ": %s : No such file or directory\n", entry);
}

#define NEEDLE_ARG argv[1]
#define DIR_ARG argv[2]

int main(int argc, char **argv)
{
	init_shm();
	if (unlikely(!stdout))
		return 1;
	if (argc == 1
	|| !argv[1][0]) {
		g_fuldirlen = 1;
		find_cat(".", 1);
		return 0;
	}
	char needlebuf[MAX_NEEDLE_LEN + 1];
	const size_t needlebuflen = init_needle(needlebuf, NEEDLE_ARG);
	init_fgrep(*needlebuf);
	init_memmem(needlebuf, needlebuflen);
	if (argc == 2)
		goto GET_CWD;
	switch (DIR_ARG[0]) {
	case '.':
		if (unlikely(DIR_ARG[1] == '\0'))
			goto GET_CWD;
	/* FALLTHROUGH */
	default:
		if (unlikely(stat(DIR_ARG, &g_st))) {
			stat_fail(DIR_ARG);
			return 1;
		}
		if (unlikely(S_ISREG(g_st.st_mode))) {
			const char *const end = strrchr(DIR_ARG, '/');
			g_fuldirlen = end ? end - DIR_ARG : 0;
			fgrep(needlebuf, DIR_ARG, needlebuflen, strlen(DIR_ARG + g_fuldirlen));
		} else if (S_ISDIR(g_st.st_mode)) {
			g_fuldirlen = 0;
			find_fgrep(needlebuf, needlebuflen, DIR_ARG, strlen(DIR_ARG));
		} else {
			no_such_file(DIR_ARG);
			return 1;
		}
		break;
	case '\0':
	GET_CWD:;
		g_fuldirlen = 1;
		find_fgrep(needlebuf, needlebuflen, ".", g_fuldirlen);
		break;
	}
	free_shm();
	return 0;
}
