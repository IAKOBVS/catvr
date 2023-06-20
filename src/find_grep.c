#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <sys/stat.h>
#include <dirent.h>

#include "librgrep.h"
#include "grep.h"
#include "config.h"

#define IF_EXCLUDED_DIR_GOTO(filename, action)               \
	do {                                                 \
		if ((filename)[0] == '.')                    \
			switch ((filename)[1]) {             \
			case '.':                            \
				/* .. */                     \
				action;                      \
				break;                       \
				/* . */                      \
			case '\0':                           \
				action;                      \
				break;                       \
				/* .git */                   \
			case 'g':                            \
				if ((filename)[2] == 'i'     \
				    && (filename)[3] == 't') \
					action;              \
				break;                       \
				/* .vscode */                \
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

#define FIND_FGREP_DO_REG(FUNC_REG, USE_LEN)                                                                \
	do {                                                                                                \
		IF_EXCLUDED_REG_GOTO(ep->d_name, goto BREAK_FIND_FGREP_DO_REG__);                           \
		if (USE_LEN)                                                                                \
			FUNC_REG(needle, fulpath, nlen, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
		else                                                                                        \
			FUNC_REG(needle, fulpath, 0, 0);                                                    \
BREAK_FIND_FGREP_DO_REG__:;                                                                                 \
	} while (0)

#define FIND_FGREP_DO_DIR(FUNC_SELF)                                                                 \
	do {                                                                                         \
		IF_EXCLUDED_DIR_GOTO(ep->d_name, goto BREAK_FIND_FGREP_DO_DIR__);                    \
		FUNC_SELF(needle, nlen, fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
BREAK_FIND_FGREP_DO_DIR__:;                                                                          \
	} while (0)

#ifdef _DIRENT_HAVE_D_TYPE

#	define IF_DIR_RECUR_IF_REG_DO(FUNC_SELF, FUNC_REG, USE_LEN)  \
		do {                                                  \
			switch (ep->d_type) {                         \
			case DT_REG:                                  \
				FIND_FGREP_DO_REG(FUNC_REG, USE_LEN); \
				break;                                \
			case DT_DIR:                                  \
				FIND_FGREP_DO_DIR(FUNC_SELF);         \
				break;                                \
			}                                             \
		} while (0)

#else

#	define IF_DIR_RECUR_IF_REG_DO(FUNC_SELF, FUNC_REG, USE_LEN)  \
		do {                                                  \
			struct stat st;                               \
			if (unlikely(stat(dir, &st)))                 \
				continue;                             \
			if (S_ISREG(st.st_mode))                      \
				FIND_FGREP_DO_REG(FUNC_REG, USE_LEN); \
			else if (S_ISDIR(g_st.st_mode))               \
				FIND_FGREP_DO_DIR(FUNC_SELF);         \
		} while (0)

#endif /* _DIRENT_HAVE_D_TYPE */

#define DEF_FIND_T(F, DO, USE_LEN)                                                                                 \
	void F(const char *RESTRICT needle, const size_t nlen, const char *RESTRICT dir, const size_t dlen) \
	{                                                                                                          \
		DIR *RESTRICT dp = opendir(dir);                                                                   \
		if (unlikely(!dp))                                                                                 \
			return;                                                                                    \
		char fulpath[MAX_PATH_LEN];                                                                        \
		struct dirent *RESTRICT ep;                                                                        \
		while ((ep = readdir(dp))) {                                                                       \
			IF_DIR_RECUR_IF_REG_DO(F, DO, USE_LEN);                                                    \
		}                                                                                                  \
		closedir(dp);                                                                                      \
		return;                                                                                            \
	}

DEF_FIND_T(find_fgrep, fgrep, 1)
