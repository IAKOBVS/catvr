#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>

#include "config.h"
#include "g_memmem.h"
#include "g_table.h"
#include "librgrep.h"
#include "macros.h"
#include "malloc.h"
#include "unlocked_io.h"

#ifndef _DIRENT_HAVE_D_TYPE
#	include "globals.h"
#endif /* _DIRENT_HAVE_D_TYPE */

void find_fgrep(const char *RESTRICT needle, const size_t nlen, const char *RESTRICT dir, const size_t dlen);

static INLINE void fgrep(const char *RESTRICT needle, const char *RESTRICT filename, const size_t nlen, const size_t flen);
NOINLINE void fgrep_noinline(const char *RESTRICT needle, const char *RESTRICT filename, const size_t nlen, const size_t flen);

NOINLINE void fgrep_noinline(const char *RESTRICT needle, const char *RESTRICT filename, const size_t nlen, const size_t flen)
{
	fgrep(needle, filename, nlen, flen);
}

#define COUNT_LINE_NUM(NL)                                          \
	do {                                                        \
		for (const unsigned char *tmp = p; tmp != linep;) { \
			switch (g_table[*tmp--]) {                  \
			case REJECT:                                \
				goto END;                           \
			case NEWLINE:                               \
				++NL;                               \
			default:;                                   \
			}                                           \
		}                                                   \
		linep = p;                                          \
	} while (0)

#if USE_LINE_NUMBER

#	define GENERATE_LINE_NUM                             \
		do {                                          \
			COUNT_LINE_NUM(NL);                   \
			numbufp = numbuf;                     \
			itoa_uint_pos(numbufp, NL, 10, dgts); \
		} while (0)

#	define PRINT_LINE_NUM \
		fwrite(numbufp, 1, dgts, stdout)

#else

#	define GENERATE_LINE_NUM
#	define PRINT_LINE_NUM

#endif /* USE_LINE_NUMBER */

#if !USE_ANSI_COLORS

#	define PRINTLN                                    \
		do {                                       \
			GENERATE_LINE_NUM;                 \
			fwrite(filename, 1, flen, stdout); \
			putchar(':');                      \
			fwrite(numbufp, 1, dgts, stdout);  \
			PRINT_LINE_NUM;                    \
			putchar(':');                      \
			fwrite(p, 1, ppp - p, stdout);     \
		} while (0)

#else

#	define PRINTLN                                                  \
		do {                                                     \
			GENERATE_LINE_NUM;                               \
			PRINT_LITERAL(ANSI_RED);                         \
			fwrite(filename, 1, flen, stdout);               \
			PRINT_LITERAL(ANSI_RESET ":");                   \
			PRINT_LITERAL(ANSI_GREEN);                       \
			PRINT_LINE_NUM;                                  \
			PRINT_LITERAL(ANSI_RESET ":");                   \
			fwrite(p, 1, pp - p, stdout);                    \
			PRINT_LITERAL(ANSI_RED);                         \
			fwrite(pp, 1, nlen, stdout);                     \
			PRINT_LITERAL(ANSI_RESET);                       \
			fwrite(pp + nlen, 1, ppp - (pp + nlen), stdout); \
		} while (0)

#endif

static INLINE void fgrep(const char *RESTRICT needle, const char *RESTRICT filename, const size_t nlen, const size_t flen)
{
	size_t sz;
	MALLOC_OPEN(filename, sz);
	unsigned char *p = g_buf;
	unsigned char *const filep = p;
	const unsigned char *linep = filep;
	const unsigned char *const pend = p + sz;
	size_t NL = 1;
	unsigned int dgts;
	char numbuf[UINT_LEN];
	char *numbufp;
	unsigned char *ppp;
	unsigned char *pp;
	for (unsigned char *lnstart; (pp = g_memmem(p, sz, needle, nlen));) {
		lnstart = p;
		for (p = pp; p != filep; --p) {
			switch (g_table[*p]) {
			case NEWLINE:
				++p;
				goto BREAK_FOR1;
			case REJECT:
				goto END;
			}
		}
BREAK_FOR1:
		for (ppp = pp + nlen;; ++ppp) {
			if (unlikely(ppp == pend))
				goto END_PRINT;
			switch (g_table[*ppp]) {
			case NEWLINE:
				++ppp;
				goto BREAK_FOR2;
			case REJECT:
				goto END;
			}
		}
BREAK_FOR2:;
		PRINTLN;
		sz -= ppp - lnstart;
		p = ppp;
	}
END:
	return;
END_PRINT:
	PRINTLN;
}

#define DO_REG(FUNC_REG)                                           \
	do {                                                       \
		IF_EXCLUDED_REG_GOTO(ep->d_name, goto CONT);       \
		flen = strlen(ep->d_name);                         \
		IF_EXCLUDED_EXT_GOTO(ep->d_name, flen, goto CONT); \
		append_len(fulpath, dir, dlen, ep->d_name, flen);  \
		FUNC_REG(needle, fulpath, nlen, dlen + flen + 1);  \
	} while (0)

#define DO_DIR(FUNC_SELF)                                                                            \
	do {                                                                                         \
		IF_EXCLUDED_DIR_GOTO(ep->d_name, goto CONT);                                         \
		FUNC_SELF(needle, nlen, fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
	} while (0)

void find_fgrep(const char *RESTRICT needle, const size_t nlen, const char *RESTRICT dir, const size_t dlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	char fulpath[MAX_PATH_LEN];
	size_t flen;
	for (struct dirent *RESTRICT ep; (ep = readdir(dp));) {
#ifdef _DIRENT_HAVE_D_TYPE
		switch (ep->d_type) {
		case DT_REG:
			DO_REG(fgrep);
			break;
		case DT_DIR:
			DO_DIR(find_fgrep);
			break;
		}
#else
		if (unlikely(stat(dir, &st)))
			continue;
		if (S_ISREG(st.st_mode))
			DO_REG(fgrep);
		else if (S_ISDIR(g_st.st_mode))
			DO_DIR(find_fgrep);
#endif /* _DIRENT_HAVE_D_TYPE */
CONT:;
	}
	closedir(dp);
	return;
}
