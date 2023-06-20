#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "librgrep.h"
#include "mmap.h"
#include "config.h"
#include "global_table_256.h"
#include "unlocked_io.h"

void cat(const char *RESTRICT filename, const size_t flen)
{
	int fd;
	size_t sz;
	unsigned char *p = mmap_open(filename, &sz, &fd);
	if (unlikely(sz >= MAX_FILE_SZ))
		return;
	if (unlikely(p == MAP_FAILED))
		return;
	const size_t filesz = sz;
	unsigned char *const filep = p;
	unsigned char *const pend = p + sz;
	char numbuf[UINT_LEN];
	char *numbufp;
	unsigned int dgts;
	if (memchr(p, 0, sz))
		goto END;
	for (unsigned int NL = 1;; ++NL) {
#if !USE_ANSI_COLORS
		fwrite(filename, 1, flen, stdout);
		putchar(':');
		numbufp = numbuf;
		itoa_uint_pos(numbufp, NL, 10, dgts);
		fwrite(numbufp, 1, dgts, stdout);
		putchar(':');
#else
		PRINT_LITERAL(ANSI_RED);
		fwrite(filename, 1, flen, stdout);
		PRINT_LITERAL(ANSI_RESET ":");
		PRINT_LITERAL(ANSI_GREEN);
		numbufp = numbuf;
		itoa_uint_pos(numbufp, NL, 10, dgts);
		fwrite(numbufp, 1, dgts, stdout);
		PRINT_LITERAL(ANSI_RESET ":");
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
			case REJECT:;
			}
			putchar(*p++);
		}
BREAK_FOR:;
	}
END:
	mmap_close(filep, filename, filesz, fd);
}
