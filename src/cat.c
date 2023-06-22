#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "config.h"
#include "g_table.h"
#include "librgrep.h"
#include "malloc.h"
#include "unlocked_io.h"

void cat(const char *RESTRICT filename, const size_t flen);

void cat(const char *RESTRICT filename, const size_t flen)
{
	size_t sz;
	MALLOC_OPEN(filename, sz);
	unsigned char *p = g_buf;
	unsigned char *const pend = p + sz;
	char numbuf[UINT_LEN];
	char *numbufp;
	unsigned int dgts;
	for (unsigned int NL = 1;; ++NL) {
#if !USE_ANSI_COLORS
		fwrite(filename, 1, flen, stdout);
		putchar(':');
#	ifdef USE_LINE_NUMBER
		numbufp = numbuf;
		itoa_uint_pos(numbufp, NL, 10, dgts);
		fwrite(numbufp, 1, dgts, stdout);
		putchar(':');
#	endif /* USE_LINE_NUMBER */
#else
		PRINT_LITERAL(ANSI_RED);
		fwrite(filename, 1, flen, stdout);
		PRINT_LITERAL(ANSI_RESET ":");
		PRINT_LITERAL(ANSI_GREEN);
#	ifdef USE_LINE_NUMBER
		numbufp = numbuf;
		itoa_uint_pos(numbufp, NL, 10, dgts);
		fwrite(numbufp, 1, dgts, stdout);
		PRINT_LITERAL(ANSI_RESET ":");
#	endif /* USE_LINE_NUMBER */
#endif
		for (;;) {
			if (unlikely(p == pend)) {
				putchar('\n');
				goto END;
			}
			switch (g_table[*p]) {
			case NEWLINE:
				++p;
				putchar('\n');
				goto BREAK_FOR;
			case REJECT:
				putchar('\n');
				goto END;
			}
			putchar(*p++);
		}
BREAK_FOR:;
	}
END:;
}
