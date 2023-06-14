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
#include <pthread.h>
#include <sys/stat.h>

#include "config.h"
#include "global_table.h"
#include "librgrep.h"
#include "unlocked_macros.h"

/* #define flockfile(fp) */
/* #define funlockfile(fp) */

#undef MAX_LINE_LEN
#define MAX_LINE_LEN 256

typedef struct fgrep_args {
	const char *needle;
	const char *filename;
	const size_t nlen;
	const size_t flen;
} fgrep_args;

static INLINE void *fgrep(void *args)
{
	fgrep_args *arg = args;
	FILE *fp = fopen(arg->filename, "r");
	if (unlikely(!fp))
		return NULL;
	char ln[MAX_LINE_LEN];
	char lnlower[MAX_LINE_LEN];
	char *lnp = ln;
	char *lnlowerp = lnlower;
	char *NLbufp;
	char NLbuf[UINT_LEN];
	const char *g_found;
	unsigned int NL = 1;
	unsigned int NLbufdigits;
	unsigned int lnlen;
	int g_c;
	int g_first_match = 0;

#define PRINT_LITERAL(s) \
	fwrite(s, 1, sizeof(s) - 1, stdout)

	do {
	CONT:
		g_c = getc(fp);
		switch (g_table[g_c + 1]) {
		case WANTED_UPPER:
			if (!g_first_match)
				g_first_match = lnp - ln;
			/* FALLTHROUGH */
		case UPPER:
			*lnp = g_c;
			*lnlowerp = g_c - 'A' + 'a';
			break;
		case WANTED:
			if (!g_first_match)
				g_first_match = lnp - ln;
			/* FALLTHROUGH */
		default:
			*lnp = g_c;
			*lnlowerp = g_c;
			break;
		case NEWLINE:
			if (g_first_match) {
#define PRINT_LN                                                                                                          \
	do {                                                                                                              \
			lnlen = lnp - ln;                                                                                 \
			if ((g_found = memmem(lnlower + g_first_match, lnlen - g_first_match, arg->needle, arg->nlen))) { \
				g_found = ln + (g_found - lnlower);                                                       \
				NLbufp = NLbuf;                                                                           \
				itoa_uint_pos(NLbufp, NL, 10, NLbufdigits);                                               \
				flockfile(stdout);                                                                        \
				PRINT_LITERAL(ANSI_RED);                                                                  \
				fwrite(arg->filename, 1, arg->flen, stdout);                                              \
				PRINT_LITERAL(ANSI_RESET ":" ANSI_GREEN);                                                 \
				fwrite(NLbufp, 1, NLbufdigits, stdout);                                                   \
				PRINT_LITERAL(ANSI_RESET ":");                                                            \
				fwrite(ln, 1, g_found - ln, stdout);                                                      \
				PRINT_LITERAL(ANSI_RED);                                                                  \
				fwrite(g_found, 1, arg->nlen, stdout);                                                    \
				PRINT_LITERAL(ANSI_RESET);                                                                \
				fwrite(g_found + arg->nlen, 1, lnlen - (g_found - ln + arg->nlen), stdout);               \
				fflush(stderr);                                                                           \
				putchar('\n');                                                                            \
				funlockfile(stdout);                                                                      \
			}                                                                                                 \
	} while (0)
				PRINT_LN;
				g_first_match = 0;
			}
			++NL;
			lnp = ln;
			lnlowerp = lnlower;
			goto CONT;
		case END_OF_FILE:
			if (g_first_match)
				PRINT_LN;
			/* FALLTHROUGH */
		case REJECT:
			goto OUT;
		}
		++lnlowerp, ++lnp;
	} while (lnp - ln != MAX_LINE_LEN);
OUT:
	fclose(fp);
	return NULL;
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

#define FIND_FGREP_DO_REG(FUNC_REG)                                                                                                             \
	do {                                                                                                                                    \
		IF_EXCLUDED_REG_DO(ep->d_name, goto CONT);                                                                                      \
		fgrep_args a = { arg->needle, ep->d_name, arg->nlen, appendp(fulpath, arg->dir, arg->dlen, ep->d_name) - fulpath - arg->dlen }; \
		FUNC_REG(&a);                                                                                                                   \
	} while (0)

#define FIND_FGREP_DO_DIR(FUNC_SELF)                                                                                             \
	do {                                                                                                                     \
		IF_EXCLUDED_DIR_DO(ep->d_name, goto CONT);                                                                       \
		find_args a = { arg->needle, arg->nlen, fulpath, appendp(fulpath, arg->dir, arg->dlen, ep->d_name) - fulpath }; \
		pthread_mutex_lock(&alive_mtx);                                                                                  \
		if (thr_alive != MAX_FORKS) {                                                                                    \
			++thr_alive;                                                                                             \
			pthread_mutex_unlock(&alive_mtx);                                                                        \
			pthread_t t;                                                                                             \
			if (unlikely(pthread_create(&t, NULL, &FUNC_SELF, &a)))                                                  \
				fputs("Can't create thread", stderr);                                                            \
		} else {                                                                                                         \
			pthread_mutex_unlock(&alive_mtx);                                                                        \
			FUNC_SELF(&a);                                                                                           \
		}                                                                                                                \
	} while (0)

unsigned int thr_alive = 0;
pthread_mutex_t alive_mtx = PTHREAD_MUTEX_INITIALIZER;

typedef struct find_args {
	const char *needle;
	const size_t nlen;
	const char *dir;
	const size_t dlen;
} find_args;

static void *find_fgrep(void *args)
{
	find_args *arg = args;
	DIR *dp = opendir(arg->dir);
	if (unlikely(!dp))
		return NULL;
	struct dirent *ep;
	char fulpath[MAX_PATH_LEN];
#ifndef _DIRENT_HAVE_D_TYPE
	struct stat st;
#endif
	while ((ep = readdir(dp))) {
#ifdef _DIRENT_HAVE_D_TYPE
		switch (ep->d_type) {
		case DT_REG:
			FIND_FGREP_DO_REG(fgrep);
#if DEBUG
			puts(ep->d_name);
#endif
			break;
		case DT_DIR:
			FIND_FGREP_DO_DIR(find_fgrep);
#if DEBUG
			puts(ep->d_name);
#endif
		}
#else
		if (unlikely(stat(dir, &st)))
			continue;
		if (S_ISREG(st.st_mode))
			FIND_FGREP_DO_REG(fgrep);
		else if (S_ISDIR(st.st_mode))
			FIND_FGREP_DO_DIR(find_fgrep);
#endif /* _DIRENT_HAVE_D_TYPE */
	CONT:;
	}
	closedir(dp);
	pthread_mutex_lock(&alive_mtx);
	--thr_alive;
	pthread_mutex_unlock(&alive_mtx);
	return args;
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
	if (argc == 1
	|| !argv[1][0]) {
		return 0;
	}
	pthread_t t;
	char needlebuf[MAX_NEEDLE_LEN + 1];
	const size_t needlebuflen = init_needle(needlebuf, NEEDLE_ARG);
	init_fgrep(*needlebuf);
	struct stat st;
	if (argc == 2)
		goto GET_CWD;
	switch (DIR_ARG[0]) {
	case '.':
		if (unlikely(DIR_ARG[1] == '\0'))
			goto GET_CWD;
	/* FALLTHROUGH */
	default:
		if (unlikely(stat(DIR_ARG, &st))) {
			stat_fail(DIR_ARG);
			return 1;
		}
		if (unlikely(S_ISREG(st.st_mode))) {
			fgrep_args args = { needlebuf, DIR_ARG, needlebuflen, strlen(DIR_ARG) };
			pthread_create(&t, NULL, &fgrep, &args);
			/* fgrep(needlebuf, DIR_ARG, needlebuflen, strlen(DIR_ARG)); */
		} else if (S_ISDIR(st.st_mode)) {
			find_args args = { needlebuf, needlebuflen, DIR_ARG, strlen(DIR_ARG) };
			pthread_create(&t, NULL, &find_fgrep, &args);
			/* find_fgrep(&args); */
		} else {
			no_such_file(DIR_ARG);
			return 1;
		}
		break;
	case '\0':
	GET_CWD: {
		find_args args = { needlebuf, needlebuflen, ".", 1 };
		pthread_create(&t, NULL, &find_fgrep, &args);
		/* find_fgrep(&args); */
		break;
		}
	}
	pthread_join(t, NULL);
	return 0;
}
