#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#define PROG_NAME "rgrep"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "config.h"
#include "g_memmem.h"
#include "global_table_256.h"
#include "librgrep.h"
#include "unlocked_macros.h"
/* #include "fork.h" */

/* 200 MB */
#define MAX_FILE_SZ 209715200

#define FORK_AND_WAIT(x) (x)
#define init_shm()
#define free_shm()

struct stat g_st;

#define itoa_uint_pos(s, n, base, digits)             \
	do {                                          \
		unsigned int n_ = n;                  \
		char *const end = (s) + UINT_LEN - 1; \
		(s) = end;                            \
		do                                    \
			*(s)-- = (n_) % (base) + '0'; \
		while ((n_) /= 10);                   \
		digits = end - (s)++;                 \
	} while (0)

#define GET_LINENUM(NL, start, end)                                   \
	do {                                                          \
		NL = 1;                                               \
		for (const unsigned char *tmp = end; tmp != start;) { \
			switch (g_table[*tmp--]) {                    \
			case NEWLINE:                                 \
				++NL;                                 \
				break;                                \
			case REJECT:                                  \
				goto END;                             \
			}                                             \
		}                                                     \
	} while (0)

#define PRINT_LITERAL(s) fwrite((s), 1, sizeof(s) - 1, stdout)

#if !USE_ANSI_COLORS

#	define FGREP_PRINT                                                         \
		do {                                                                \
			for (const unsigned char *tmp = plinestart;; ++tmp, ++NL) { \
				tmp = memchr(tmp, '\n', pp - tmp);                  \
				if (unlikely(!tmp))                                 \
					break;                                      \
				plinestart = tmp;                                   \
			}                                                           \
                                                                                    \
			numbufp = numbuf;                                           \
			itoa_uint_pos(numbufp, NL, 10, dgts);                       \
                                                                                    \
			fwrite(filename, 1, flen, stdout);                          \
			putchar(':');                                               \
                                                                                    \
			fwrite(numbufp, 1, dgts, stdout);                           \
			putchar(':');                                               \
			fwrite(p, 1, ppp - p, stdout);                              \
		} while (0)

#else

#	define FGREP_PRINT                                                         \
		do {                                                                \
			for (const unsigned char *tmp = plinestart;; ++tmp, ++NL) { \
				tmp = memchr(tmp, '\n', pp - tmp);                  \
				if (unlikely(!tmp))                                 \
					break;                                      \
				plinestart = tmp;                                   \
			}                                                           \
			numbufp = numbuf;                                           \
			itoa_uint_pos(numbufp, NL, 10, dgts);                       \
			PRINT_LITERAL(ANSI_RED);                                    \
			fwrite(filename, 1, flen, stdout);                          \
			PRINT_LITERAL(ANSI_RESET ":");                              \
			PRINT_LITERAL(ANSI_GREEN);                                  \
			fwrite(numbufp, 1, dgts, stdout);                           \
			PRINT_LITERAL(ANSI_RESET ":");                              \
			fwrite(p, 1, pp - p, stdout);                               \
			PRINT_LITERAL(ANSI_RED);                                    \
			fwrite(pp, 1, needlelen, stdout);                           \
			PRINT_LITERAL(ANSI_RESET);                                  \
			fwrite(pp + needlelen, 1, ppp - (pp + needlelen), stdout);  \
		} while (0)

#endif

static INLINE void fgrep(const char *needle, const char *filename, const size_t needlelen, const size_t flen)
{
	int fd = open(filename, O_RDONLY, S_IRUSR);
	if (unlikely(fd < 0
		     || fstat(fd, &g_st)
		     || g_st.st_size >= MAX_FILE_SZ
		     || !g_st.st_size))
		return;
	const unsigned int csz = g_st.st_size;
	unsigned int sz = csz;
	unsigned char *p = mmap(NULL, csz + 1, PROT_READ, MAP_PRIVATE, fd, 0);
	unsigned char *const pstart = (unsigned char *)p;
	unsigned int NL = 1;
	unsigned int dgts;
	char numbuf[UINT_LEN];
	char *numbufp;
	const unsigned char *plinestart = pstart;
	const unsigned char *const pend = p + sz;
	unsigned char *pp;
	unsigned char *ppp;
	while ((pp = (unsigned char *)g_memmem(p, sz, needle, needlelen))) {
		p = pp;
		for (;;) {
			switch (g_table[*p]) {
			case NEWLINE:
				++p;
				goto BREAK_FOR1;
			case REJECT:
				goto END;
			}
			if (unlikely(p == pstart))
				break;
			--p;
		}
BREAK_FOR1:
		ppp = pp + needlelen;
		for (;;) {
			if (unlikely(ppp == pend)) {
				FGREP_PRINT;
				goto END;
			}
			switch (g_table[*ppp]) {
			case NEWLINE:
				++ppp;
				goto BREAK_FOR2;
			case REJECT:
				goto END;
			}
			++ppp;
		}
BREAK_FOR2:;
		FGREP_PRINT;
		sz -= ppp - p;
		p = ppp;
	}
END:;
	if (unlikely(munmap(pstart, csz + 1))) {
		fprintf(stderr, "Can't munmap %s\n", filename);
		exit(1);
	}
	exit(1);
}

#define IF_EXCLUDED_REG_DO(filename, action)  \
	do {                                  \
		if ((filename)[0] == '.'      \
		    && (filename)[1] == 'c'   \
		    && (filename)[2] == 'l'   \
		    && (filename)[3] == 'a'   \
		    && (filename)[4] == 'n'   \
		    && (filename)[5] == 'g'   \
		    && (filename)[6] == '-'   \
		    && (filename)[7] == 'f'   \
		    && (filename)[8] == 'o'   \
		    && (filename)[9] == 'r'   \
		    && (filename)[10] == 'm'  \
		    && (filename)[11] == 'a'  \
		    && (filename)[12] == 't') \
			action;               \
	} while (0)

/* skip . , .., .git, .vscode */
#define IF_EXCLUDED_DIR_DO(filename, action)                 \
	do {                                                 \
		if ((filename)[0] == '.')                    \
			switch ((filename)[1]) {             \
			case '.':                            \
			case '\0':                           \
				action;                      \
				break;                       \
			case 'g':                            \
				if ((filename)[2] == 'i'     \
				    && (filename)[3] == 't') \
					action;              \
				break;                       \
			case 'v':                            \
				if ((filename)[2] == 's'     \
				    && (filename)[3] == 'c'  \
				    && (filename)[4] == 'o'  \
				    && (filename)[5] == 'd'  \
				    && (filename)[6] == 'e') \
					action;              \
				break;                       \
			}                                    \
	} while (0)

#define FIND_FGREP_DO_REG(FUNC_REG, USE_LEN)                                                                     \
	do {                                                                                                     \
		IF_EXCLUDED_REG_DO(ep->d_name, goto CONT);                                                       \
		if (USE_LEN)                                                                                     \
			FUNC_REG(needle, fulpath, needlelen, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
		else                                                                                             \
			FUNC_REG(needle, fulpath, 0, 0);                                                         \
	} while (0)

#define FIND_FGREP_DO_DIR(FUNC_SELF)                                                                      \
	do {                                                                                              \
		IF_EXCLUDED_DIR_DO(ep->d_name, goto CONT);                                                \
		FUNC_SELF(needle, needlelen, fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
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
CONT:;                                                                                                \
		}                                                                                     \
		closedir(dp);                                                                         \
		return;                                                                               \
	}

DEF_FIND_T(find_fgrep, fgrep, 1)

static INLINE void cat(const char *RESTRICT filename, const size_t flen)
{
	int fd = open(filename, O_RDONLY, S_IRUSR);
	if (unlikely(fd < 0)
	    || unlikely(fstat(fd, &g_st))
	    || unlikely(g_st.st_size >= MAX_FILE_SZ)
	    || unlikely(!g_st.st_size))
		return;
	const unsigned int sz = g_st.st_size;
	unsigned char *p = mmap(NULL, sz, PROT_READ, MAP_PRIVATE, fd, 0);
	unsigned char *const pstart = p;
	unsigned char *const pend = p + sz;
	char numbuf[UINT_LEN];
	char *numbufp;
	unsigned int dgts;
	if (!memchr(p, 0, sz)) {
		for (unsigned int NL = 1;; ++NL) {
#if !USE_ANSI_COLORS
			fwrite(filename, 1, flen, stdout);
			putchar(':');

			numbufp = numbuf;
			itoa_uint_pos(numbufp, NL, 10, dgts);
			fwrite(numbufp, 1, dgts, stdout);

			putchar(':');
			for (;;) {
				switch (g_table[*p]) {
				case NEWLINE:
					++p;
					putchar('\n');
					goto BREAK_FOR;
				case REJECT:;
				}
				if (unlikely(p == pend)) {
					putchar('\n');
					goto END;
				}
				putchar(*p++);
			}
#else
			PRINT_LITERAL(ANSI_RED);
			fwrite(filename, 1, flen, stdout);
			PRINT_LITERAL(ANSI_RESET ":");
			PRINT_LITERAL(ANSI_GREEN);

			numbufp = numbuf;
			itoa_uint_pos(numbufp, NL, 10, dgts);
			fwrite(numbufp, 1, dgts, stdout);

			PRINT_LITERAL(ANSI_RESET ":");
			for (;;) {
				switch (g_table[*p]) {
				case NEWLINE:
					++p;
					putchar('\n');
					goto BREAK_FOR;
				case REJECT:;
				}
				if (unlikely(p == pend)) {
					putchar('\n');
					goto END;
				}
				putchar(*p++);
			}
#endif
BREAK_FOR:;
		}
	}
END:
	if (unlikely(munmap(pstart, sz))) {
		fprintf(stderr, "Can't munmap %s\n", filename);
		exit(1);
	}
}

static void find_cat(const char *RESTRICT dir, const size_t dlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	struct dirent *RESTRICT ep;
	char fulpath[MAX_PATH_LEN];

#define FIND_CAT_DO_REG                                                          \
	do {                                                                     \
		IF_EXCLUDED_REG_DO(ep->d_name, goto CONT);                       \
		cat(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
	} while (0)

#define FIND_CAT_DO_DIR                                                               \
	do {                                                                          \
		IF_EXCLUDED_DIR_DO(ep->d_name, goto CONT);                            \
		find_cat(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
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

#define NEEDLELEN strlen(NEEDLE_ARG)

int main(int argc, char **argv)
{
	init_shm();
	if (argc == 1 || !argv[1][0]) {
		find_cat(".", 1);
		return 1;
	}
	const size_t needlelen = strlen(NEEDLE_ARG);
	init_memmem(NEEDLE_ARG, needlelen);
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
			fgrep(NEEDLE_ARG, DIR_ARG, needlelen, strlen(DIR_ARG));
		} else if (S_ISDIR(g_st.st_mode)) {
			find_fgrep(NEEDLE_ARG, needlelen, DIR_ARG, strlen(DIR_ARG));
		} else {
			no_such_file(DIR_ARG);
			return 1;
		}
		break;
	case '\0':
GET_CWD:;
		find_fgrep(NEEDLE_ARG, needlelen, ".", 1);
		break;
	}
	free_shm();
	return 0;
}
