#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <stddef.h>

#include "config.h"
#include "g_table.h"
#include "librgrep.h"
#include "malloc.h"
#include "unlocked_io.h"

static INLINE void cat(const char *RESTRICT filename, const size_t flen);

static INLINE void cat(const char *RESTRICT filename, const size_t flen)
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

void find_cat(const char *RESTRICT dir, const size_t dlen);

void find_cat(const char *RESTRICT dir, const size_t dlen)
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
#	define DO_REG(FUNC_REG)                                   \
		IF_EXCLUDED_REG_GOTO(ep->d_name, goto CONT);       \
		flen = strlen(ep->d_name);                         \
		IF_EXCLUDED_EXT_GOTO(ep->d_name, flen, goto CONT); \
		append_len(fulpath, dir, dlen, ep->d_name, flen);  \
		FUNC_REG(fulpath, dlen + flen + 1);
			DO_REG(cat);
			break;
		case DT_DIR:
#	define DO_DIR(FUNC_SELF)                            \
		IF_EXCLUDED_DIR_GOTO(ep->d_name, goto CONT); \
		FUNC_SELF(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath);
			DO_DIR(find_cat);
			break;
		}
#else
		if (unlikely(stat(dir, &st)))
			continue;
		if (S_ISREG(st.st_mode))
			DO_REG(cat);
		else if (S_ISDIR(st.st_mode))
			DO_DIR(find_cat);
#endif /* _DIRENT_HAVE_D_TYPE */
CONT:;
	}
	closedir(dp);
}
