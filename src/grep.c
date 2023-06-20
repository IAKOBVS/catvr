#include <stddef.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "config.h"
#include "g_memmem.h"
#include "global_table_256.h"
#include "librgrep.h"
#include "mmap.h"
#include "unlocked_io.h"

#define COUNT_NL(NL)                                                \
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

#if !USE_ANSI_COLORS

#	define PRINTLN                                       \
		do {                                          \
			COUNT_NL(NL);                         \
			numbufp = numbuf;                     \
			itoa_uint_pos(numbufp, NL, 10, dgts); \
			fwrite(filename, 1, flen, stdout);    \
			putchar(':');                         \
			fwrite(numbufp, 1, dgts, stdout);     \
			putchar(':');                         \
			fwrite(p, 1, ppp - p, stdout);        \
		} while (0)

#else

#	define PRINTLN                                                  \
		do {                                                     \
			COUNT_NL(NL);                                    \
			numbufp = numbuf;                                \
			itoa_uint_pos(numbufp, NL, 10, dgts);            \
			PRINT_LITERAL(ANSI_RED);                         \
			fwrite(filename, 1, flen, stdout);               \
			PRINT_LITERAL(ANSI_RESET ":");                   \
			PRINT_LITERAL(ANSI_GREEN);                       \
			fwrite(numbufp, 1, dgts, stdout);                \
			PRINT_LITERAL(ANSI_RESET ":");                   \
			fwrite(p, 1, pp - p, stdout);                    \
			PRINT_LITERAL(ANSI_RED);                         \
			fwrite(pp, 1, nlen, stdout);                     \
			PRINT_LITERAL(ANSI_RESET);                       \
			fwrite(pp + nlen, 1, ppp - (pp + nlen), stdout); \
		} while (0)

#endif

void fgrep(const char *RESTRICT needle, const char *RESTRICT filename, const size_t nlen, const size_t flen)
{
	int fd;
	size_t sz;
	unsigned char *p = mmap_open(filename, &sz, &fd);
	if (unlikely(sz == MAX_FILE_SZ))
		return;
	if (unlikely(p == MAP_FAILED)) {
		if (!sz)
			return;
		fgrep_err("Mmap failed", filename);
		exit(1);
		return;
	}
	const size_t filesz = sz;
	unsigned char *const filep = p;
	const unsigned char *linep = filep;
	const unsigned char *const pend = p + sz;
	size_t NL = 1;
	unsigned int dgts;
	char numbuf[UINT_LEN];
	char *numbufp;
	for (unsigned char *lnstart, *pp, *ppp; (pp = (unsigned char *)g_memmem(p, sz, needle, nlen));) {
		lnstart = p;
		p = pp;
		while (p != filep) {
			switch (g_table[*p]) {
			case NEWLINE:
				++p;
				goto BREAK_FOR1;
			case REJECT:
				goto END;
			}
			--p;
		}
BREAK_FOR1:
		ppp = pp + nlen;
		for (;;) {
			if (unlikely(ppp == pend)) {
				PRINTLN;
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
		PRINTLN;
		sz -= ppp - lnstart;
		p = ppp;
	}
END:;
	mmap_close(filep, filename, filesz, fd);
}
