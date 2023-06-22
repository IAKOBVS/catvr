#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <stddef.h>

#include "cat.h"
#include "config.h"
#include "librgrep.h"

#define DO_REG                                                                   \
	do {                                                                     \
		IF_EXCLUDED_REG_GOTO(ep->d_name, goto CONT);                     \
		cat(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
	} while (0)

#define DO_DIR                                                                        \
	do {                                                                          \
		IF_EXCLUDED_DIR_GOTO(ep->d_name, goto CONT);                          \
		find_cat(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
	} while (0)

void find_cat(const char *RESTRICT dir, const size_t dlen);

void find_cat(const char *RESTRICT dir, const size_t dlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	char fulpath[MAX_PATH_LEN];
	for (struct dirent *RESTRICT ep; (ep = readdir(dp));) {
#ifdef _DIRENT_HAVE_D_TYPE
		switch (ep->d_type) {
		case DT_REG:
			DO_REG;
			break;
		case DT_DIR:
			DO_DIR;
			break;
		}
#else
		if (unlikely(stat(dir, &st)))
			continue;
		if (S_ISREG(st.st_mode))
			DO_REG;
		else if (S_ISDIR(st.st_mode))
			DO_DIR;
#endif /* _DIRENT_HAVE_D_TYPE */
CONT:;
	}
	closedir(dp);
}