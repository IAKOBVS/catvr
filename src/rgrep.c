#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include "globals.h"
#include "g_memmem.h"
#include "librgrep.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "unlocked_macros.h"

static INLINE void fgrep(const char *ptn, const char *filename, const size_t ptnlen, const size_t flen)
{
	FILE *fp = fopen(filename, "r");
	if (unlikely(!fp))
		return;
	g_first_match = 0;
	g_NL = 1;
	g_lnp = g_ln;
	g_lnlowerp = g_lnlower;
	filename = filename + g_fuldirlen + 1;

#define CPY_N_ADV(dst, src)                        \
	do {                                       \
		memcpy(dst, src, sizeof(src) - 1); \
		(dst) += (sizeof(src) - 1);        \
	} while (0)

#define CPY_N_ADV_LEN(dst, src, n)   \
	do {                         \
		memcpy(dst, src, n); \
		(dst) += (n);        \
	} while (0)

#define PRINT_LITERAL(s) \
	fwrite(s, 1, sizeof(s) - 1, stdout)

#define PRINT_LN(i)                                                                                          \
	do {                                                                                                 \
		g_lnlen = (g_lnp + i) - g_ln + 1;                                                            \
		if ((g_found = g_memmem(g_lnlower + g_first_match, g_lnlen - g_first_match, ptn, ptnlen))) { \
			g_found = g_ln + (g_found - g_lnlower);                                              \
			g_NLbufp = g_NLbuf;                                                                  \
			itoa_uint_pos(g_NLbufp, g_NL, 10, g_NLbufdigits);                                    \
			flockfile(stdout);                                                                   \
			PRINT_LITERAL(ANSI_RED);                                                             \
			fwrite(filename, 1, flen, stdout);                                                   \
			PRINT_LITERAL(ANSI_RESET ":" ANSI_GREEN);                                            \
			fwrite(g_NLbufp, 1, g_NLbufdigits, stdout);                                          \
			PRINT_LITERAL(ANSI_RESET ":");                                                       \
			fwrite(g_ln, 1, g_found - g_ln, stdout);                                             \
			PRINT_LITERAL(ANSI_RED);                                                             \
			fwrite(g_found, 1, ptnlen, stdout);                                                  \
			PRINT_LITERAL(ANSI_RESET);                                                           \
			fwrite(g_found + ptnlen, 1, g_lnlen - (g_found - g_ln + ptnlen), stdout);            \
			funlockfile(stdout);                                                                 \
		}                                                                                            \
	} while (0)

	do {

#define LOOP_FGREP(i)                                             \
	do {                                                      \
		g_c = getc(fp);                                   \
		switch (g_table[g_c + 1]) {                       \
		case WANTED_UPPER:                                \
			if (!g_first_match)                       \
				g_first_match = g_lnp + i - g_ln; \
			/* FALLTHROUGH */                         \
		case UPPER:                                       \
			g_lnp[i] = g_c;                           \
			g_lnlowerp[i] = g_c - 'A' + 'a';          \
			break;                                    \
		case WANTED:                                      \
			if (!g_first_match)                       \
				g_first_match = g_lnp + i - g_ln; \
			/* FALLTHROUGH */                         \
		default:                                          \
			g_lnp[i] = g_c;                           \
			g_lnlowerp[i] = g_c;                      \
			break;                                    \
		case NEWLINE:                                     \
			g_lnp[i] = '\n';                          \
			if (g_first_match) {                      \
				PRINT_LN(i);                      \
				g_first_match = 0;                \
			}                                         \
			++g_NL;                                   \
			g_lnp = g_ln;                             \
			g_lnlowerp = g_lnlower;                   \
			goto CONT;                                \
		case END_OF_FILE:                                 \
			g_lnp[i] = '\n';                          \
			if (g_first_match)                        \
				PRINT_LN(i);                      \
			/* FALLTHROUGH */                         \
		case REJECT:                                      \
			goto OUT;                                 \
		}                                                 \
	} while (0)

		LOOP_FGREP(0);
		LOOP_FGREP(1);
		LOOP_FGREP(2);
		LOOP_FGREP(3);
		g_lnp += 4, g_lnlowerp += 4;
	CONT:;
	} while (MAX_LINE_LEN - 4 > (g_lnp - g_ln));
OUT:
	fclose(fp);
}

/* skip . , .., .git, .vscode */
#define IF_EXCLUDED_DO(filename, action)       \
	if (filename[0] == '.')                \
		switch (filename[1]) {         \
		case '.':                      \
		case '\0':                     \
			action;                \
			break;                 \
		case 'g':                      \
			if (filename[2] == 'i' \
			&& filename[3] == 't') \
				action;        \
			break;                 \
		case 'v':                      \
			if (filename[2] == 's' \
			&& filename[3] == 'c'  \
			&& filename[4] == 'o'  \
			&& filename[5] == 'd'  \
			&& filename[6] == 'e') \
				action;        \
			break;                 \
		}

#define FIND_FGREP_DO_REG(FUNC_REG, USE_LEN)                                                                           \
	if (USE_LEN)                                                                                                   \
		FUNC_REG(ptn, fulpath, ptnlen, appendp(fulpath, dir, dlen, ep->d_name) - (fulpath + g_fuldirlen) - 1); \
	else                                                                                                           \
		FUNC_REG(ptn, fulpath, 0, 0)

#define FIND_FGREP_DO_DIR(FUNC_SELF)                                                                \
	IF_EXCLUDED_DO(ep->d_name, continue)                                                        \
	if (pid > 0) {                                                                              \
		if (g_child_tot == g_child_max) {                                                   \
			wait(NULL);                                                                 \
			--g_child_tot;                                                              \
		}                                                                                   \
		pid = fork();                                                                       \
	}                                                                                           \
	switch (pid) {                                                                              \
	case 0:                                                                                     \
		FUNC_SELF(ptn, ptnlen, fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
		break;                                                                              \
	default:                                                                                    \
		++g_child_tot;                                                                      \
	case -1:;                                                                                   \
	}

#ifdef _DIRENT_HAVE_D_TYPE

#	define IF_DIR_RECUR_IF_REG_DO(FUNC_SELF, FUNC_REG, USE_LEN) \
		switch (ep->d_type) {                                \
		case DT_REG:                                         \
			FIND_FGREP_DO_REG(FUNC_REG, USE_LEN);        \
			break;                                       \
		case DT_DIR:                                         \
			/* skip . , .., .git, .vscode */             \
			FIND_FGREP_DO_DIR(FUNC_SELF);                \
		}

#else

#	define IF_DIR_RECUR_IF_REG_DO(FUNC_SELF, FUNC_REG, USE_LEN) \
		if (unlikely(stat(dir, &g_st)))                      \
			continue;                                    \
		if (S_ISREG(g_st.st_mode)) {                         \
			FIND_FGREP_DO_REG(FUNC_REG, USE_LEN);        \
		} else if (S_ISDIR(g_st.st_mode)) {                  \
			FIND_FGREP_DO_DIR(FUNC_SELF);                \
		}

#endif /* _DIRENT_HAVE_D_TYPE */

#define DEF_FIND_T(F, DO, USE_LEN)                                                              \
	static void F(const char *ptn, const size_t ptnlen, const char *dir, const size_t dlen) \
	{                                                                                       \
		DIR *dp = opendir(dir);                                                         \
		if (unlikely(!dp))                                                              \
			return;                                                                 \
		struct dirent *ep;                                                              \
		char fulpath[MAX_PATH_LEN];                                                     \
		while ((ep = readdir(dp))) {                                                    \
			IF_DIR_RECUR_IF_REG_DO(F, DO, USE_LEN)                                  \
		}                                                                               \
		closedir(dp);                                                                   \
		return ;                                                                        \
	}

DEF_FIND_T(find_fgrep, fgrep, 1)

static INLINE void catv(const char *RESTRICT filename, const size_t flen)
{
	FILE *RESTRICT fp = fopen(filename, "r");
	if (unlikely(!fp))
		return;
	filename = filename + g_fuldirlen + 1;
	g_lnp = g_ln;
	CPY_N_ADV(g_lnp, ANSI_RED);
	CPY_N_ADV_LEN(g_lnp, filename, flen);
	CPY_N_ADV(g_lnp, ANSI_RESET ":" ANSI_GREEN "1" ANSI_RESET ":");
	g_NL = 2;

#define LOOP_CAT(i)                                                     \
	switch (g_lnp[i] = getc(fp)) {                                  \
	default:                                                        \
	case '\t':                                                      \
		break;                                                  \
	case '\n':                                                      \
		g_NLbufp = g_NLbuf;                                     \
		itoa_uint_pos(g_NLbufp, g_NL, 10, g_NLbufdigits);       \
		flockfile(stdout);                                      \
		fwrite(g_ln, 1, (g_lnp + i) - g_ln + 1, stdout);        \
		PRINT_LITERAL(ANSI_RED);                                \
		fwrite(filename, 1, flen, stdout);                      \
		PRINT_LITERAL(ANSI_RESET ":" ANSI_GREEN);               \
		fwrite(g_NLbufp, 1, g_NLbufdigits, stdout);             \
		PRINT_LITERAL(ANSI_RESET ":");                          \
		funlockfile(stdout);                                    \
		++g_NL;                                                 \
		g_lnp = g_ln;                                           \
		goto CONT;                                              \
	case EOF:                                                       \
		g_lnp[i] = '\n';                                        \
		fwrite_locked(g_ln, 1, (g_lnp + i) - g_ln + 1, stdout); \
	case '\0':                                                      \
		CASE_UNPRINTABLE_WO_NUL_TAB_NL                          \
		goto OUT;                                               \
	}

	do {
		LOOP_CAT(0);
		LOOP_CAT(1);
		LOOP_CAT(2);
		LOOP_CAT(3);
		g_lnp += 4;
	CONT:;
	} while (MAX_LINE_LEN - 4 > (g_lnp - g_ln));
OUT:
	fclose(fp);
}

static void find_cat(const char *RESTRICT dir, const size_t dlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	struct dirent *RESTRICT ep;
	char fulpath[MAX_PATH_LEN];
	while ((ep = readdir(dp))) {
#if DEBUG
		printf("d->name: %s\n", ep->d_name);
#endif /* DEBUG */

#define FIND_CAT_DO_REG \
	catv(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - (fulpath + g_fuldirlen) - 1)

#define FIND_CAT_DO_DIR                                                               \
	IF_EXCLUDED_DO(ep->d_name, continue)                                          \
	if (pid > 0) {                                                                \
		if (g_child_tot == g_child_max) {                                     \
			wait(NULL);                                                   \
			--g_child_tot;                                                \
		}                                                                     \
		pid = fork();                                                         \
	}                                                                             \
	switch (pid) {                                                                \
	case 0:                                                                       \
		find_cat(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath); \
		break;                                                                \
	default:                                                                      \
		++g_child_tot;                                                        \
	case -1:;                                                                     \
	}

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
			return;
		if (S_ISREG(g_st.st_mode))
			FIND_CAT_DO_REG;
		else if (S_ISDIR(g_st.st_mode))
			FIND_CAT_DO_DIR;
#endif /* _DIRENT_HAVE_D_TYPE */
#if DEBUG
		printf("entries: %s\n", ep->d_name);
#endif /* DEBUG */
	}
	closedir(dp);
}

static void get_dir(char *buf)
{
	getcwd(buf, MAX_PATH_LEN);
	g_fuldirlen = strlen(buf);
}

static INLINE void set_pattern(char *dst, const char *src)
{
	for (;; ++src, ++dst) {
		switch (*src) {
			CASE_UPPER
			*dst = *src - 'A' + 'a';
			continue;
		default:
			*dst = *src;
			continue;
		case '\0':
			*dst = '\0';
		}
		break;
	}
}

static INLINE void init_table(char ptn)
{
	g_table[(unsigned char)ptn + 1] = WANTED;
	g_table[(unsigned char)ptn - 'a' + 'A' + 1] = WANTED_UPPER;
}

#define PTN_ argv[1]
#define DIR_ argv[2]

int main(int argc, char **argv)
{
	g_child_max = sysconf(_SC_NPROCESSORS_CONF);
	if (unlikely(g_child_max == -1)) {
		fputs("Can't get number of cores!", stderr);
		return EXIT_FAILURE;
	}
	if (argc == 1 || !argv[1][0]) {
		char cwd[MAX_PATH_LEN];
		get_dir(cwd);
		find_cat(cwd, g_fuldirlen);
		return EXIT_SUCCESS;
	}
	char ptn[MAX_ARG_LEN];
	set_pattern(ptn, PTN_);
	init_table(*ptn);
	init_memmem_table(ptn);
	if (argc == 2)
		goto GET_CWD;
	switch (DIR_[0]) {
	case '.':
		if (unlikely(DIR_[1] == '\0'))
			goto GET_CWD;
	/* FALLTHROUGH */
	default:
		if (unlikely(stat(DIR_, &g_st))) {
			printf("%s not a valid file or dir\n", DIR_);
			return 1;
		}
		if (unlikely(S_ISREG(g_st.st_mode))) {
			g_fuldirlen = strrchr(DIR_, '/') - DIR_;
			fgrep(ptn, DIR_, strlen(ptn), strlen(DIR_ + g_fuldirlen));
		} else {
			g_fuldirlen = strlen(DIR_);
			find_fgrep(ptn, strlen(ptn), DIR_, g_fuldirlen);
		}
		break;
	case '\0':
	GET_CWD:;
		char cwd[MAX_PATH_LEN];
		get_dir(cwd);
		find_fgrep(ptn, strlen(ptn), cwd, g_fuldirlen);
		break;
	}
	return 0;
}
