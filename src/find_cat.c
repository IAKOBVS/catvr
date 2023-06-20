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

void find_cat(const char *RESTRICT dir, const size_t dlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	struct dirent *RESTRICT ep;
	char fulpath[MAX_PATH_LEN];

#define FIND_CAT_DO_REG                                                          \
	do {                                                                     \
		IF_EXCLUDED_REG_GOTO(ep->d_name, goto CONT);                     \
		cat(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
	} while (0)

#define FIND_CAT_DO_DIR                                                               \
	do {                                                                          \
		IF_EXCLUDED_DIR_GOTO(ep->d_name, goto CONT);                          \
		find_cat(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
	} while (0)

	while ((ep = readdir(dp))) {
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
CONT:;
	}
	closedir(dp);
}
