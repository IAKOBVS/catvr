#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <sys/stat.h>

#include "config.h"
#include "grep.h"
#include "librgrep.h"

#define DO_REG(FUNC_REG)                                                                            \
	do {                                                                                        \
		IF_EXCLUDED_REG_GOTO(ep->d_name, goto CONT);                                        \
		FUNC_REG(needle, fulpath, nlen, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
	} while (0)

#define DO_DIR(FUNC_SELF)                                                                            \
	do {                                                                                         \
		IF_EXCLUDED_DIR_GOTO(ep->d_name, goto CONT);                                         \
		FUNC_SELF(needle, nlen, fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
	} while (0)

#ifdef _DIRENT_HAVE_D_TYPE

#	define DECLARE_STAT struct stat
#	define GREP_IF_REG(FUNC_SELF, FUNC_REG)   \
		do {                               \
			switch (ep->d_type) {      \
			case DT_REG:               \
				DO_REG(FUNC_REG);  \
				break;             \
			case DT_DIR:               \
				DO_DIR(FUNC_SELF); \
				break;             \
			}                          \
		} while (0)

#else

#	define DECLARE_STAT
#	define GREP_IF_REG(FUNC_SELF, FUNC_REG)        \
		do {                                    \
			if (unlikely(stat(dir, &st)))   \
				continue;               \
			if (S_ISREG(st.st_mode))        \
				DO_REG(FUNC_REG);       \
			else if (S_ISDIR(g_st.st_mode)) \
				DO_DIR(FUNC_SELF);      \
		} while (0)

#endif /* _DIRENT_HAVE_D_TYPE */

#define DEF_FIND_T(FUNC_SELF, FUNC_REG)                                                                              \
	void FUNC_SELF(const char *RESTRICT needle, const size_t nlen, const char *RESTRICT dir, const size_t dlen); \
	void FUNC_SELF(const char *RESTRICT needle, const size_t nlen, const char *RESTRICT dir, const size_t dlen)  \
	{                                                                                                            \
		DIR *RESTRICT dp = opendir(dir);                                                                     \
		if (unlikely(!dp))                                                                                   \
			return;                                                                                      \
		char fulpath[MAX_PATH_LEN];                                                                          \
		DECLARE_STAT;                                                                                        \
		for (struct dirent * RESTRICT ep; (ep = readdir(dp));) {                                             \
			GREP_IF_REG(FUNC_SELF, FUNC_REG);                                                            \
CONT:;                                                                                                               \
		}                                                                                                    \
		closedir(dp);                                                                                        \
		return;                                                                                              \
	}

DEF_FIND_T(find_fgrep, fgrep)

/* #define DO_REG(FUNC_REG)                                                                                    \ */
/* 	do {                                                                                                \ */
/* 		IF_EXCLUDED_REG_GOTO(ep->d_name, goto BREAK_DO_REG__);                                      \ */
/* 		if (USE_LEN)                                                                                \ */
/* 			FUNC_REG(needle, fulpath, nlen, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \ */
/* 		else                                                                                        \ */
/* 			FUNC_REG(needle, fulpath, 0, 0);                                                    \ */
/* BREAK_DO_REG__:;                                                                                            \ */
/* 	} while (0) */
