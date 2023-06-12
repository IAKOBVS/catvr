#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "../lib/libcatvr.h"
#include <unistd.h>

#ifndef UINT_LEN
#	define UINT_LEN 10
#endif /* UINT_LEN */
#define MAX_LINE_LEN 512
#define MAX_PATH_LEN 4096
#define MAX_ARG_LEN 256

int g_lnlen;
char g_ln[MAX_LINE_LEN];
char g_lnlower[MAX_LINE_LEN];
char *g_lnlowerp;
char *g_lnp;
char g_NLbuf[UINT_LEN];
char *g_NLbufp;
unsigned int g_NL;
unsigned int g_NLbufdigits;
int g_fuldirlen;
int g_c;
const char *g_found;
struct stat g_st;

#ifdef HAS_FGETC_UNLOCKED
#	define fgetc(c) fgetc_unlocked(c)
#endif

#ifdef HAS_GETC_UNLOCKED
#	define getc(fp) getc_unlocked(fp)
#endif

#ifdef HAS_PUTCHAR_UNLOCKED
#	define putchar(c) putchar_unlocked(c)
#endif

#ifdef HAS_FGETS_UNLOCKED
#	define fgets(s, N, fp) fgets_unlocked(s, N, fp)
#endif

#ifdef HAS_FWRITE_UNLOCKED
#	define fwrite(s, sz, N, fp) fwrite_unlocked(s, sz, N, fp)
#	define fwrite_locked(s, sz, N, fp) fwrite(s, sz, N, fp)
#else
#	define fwrite_locked(s, sz, N, fp) fwrite(s, sz, N, fp)
#	define flockfile(fp)
#	define funlockfile(fp)
#endif

#ifndef HAS_MEMMEM
#	define memmem(haystack, haystacklen, needle, needlelen) strstr(haystack, needle)
#endif /* !HAS_MEMMEM */

int g_wanted;
enum { ACCEPT = 0, REJECT, NEWLINE, UPPER, LOWER, WANTED, WANTED_UPPER, FILLED, END_OF_FILE };
unsigned char g_table[257] = {

	/* EOF */
	END_OF_FILE, /* -1 */

	/* unprintable */
	REJECT, /* 0 */
	REJECT, /* 1 */
	REJECT, /* 2 */
	REJECT, /* 3 */
	REJECT, /* 4 */
	REJECT, /* 5 */
	REJECT, /* 6 */
	REJECT, /* 7 */
	REJECT, /* 8 */

	/* tab and newline */
	ACCEPT, /* 9 */
	NEWLINE, /* 10 */

	REJECT, /* 11 */
	REJECT, /* 12 */
	REJECT, /* 13 */
	REJECT, /* 14 */
	REJECT, /* 15 */
	REJECT, /* 16 */
	REJECT, /* 17 */
	REJECT, /* 18 */
	REJECT, /* 19 */
	REJECT, /* 20 */
	REJECT, /* 21 */
	REJECT, /* 22 */
	REJECT, /* 23 */
	REJECT, /* 24 */
	REJECT, /* 25 */
	REJECT, /* 26 */
	REJECT, /* 27 */
	REJECT, /* 28 */
	REJECT, /* 29 */
	REJECT, /* 30 */
	REJECT, /* 31 */

	/* printable */
	ACCEPT, /* 32 */
	ACCEPT, /* 33 */
	ACCEPT, /* 34 */
	ACCEPT, /* 35 */
	ACCEPT, /* 36 */
	ACCEPT, /* 37 */
	ACCEPT, /* 38 */
	ACCEPT, /* 39 */
	ACCEPT, /* 40 */
	ACCEPT, /* 41 */
	ACCEPT, /* 42 */
	ACCEPT, /* 43 */
	ACCEPT, /* 44 */
	ACCEPT, /* 45 */
	ACCEPT, /* 46 */
	ACCEPT, /* 47 */
	ACCEPT, /* 48 */
	ACCEPT, /* 49 */
	ACCEPT, /* 50 */
	ACCEPT, /* 51 */
	ACCEPT, /* 52 */
	ACCEPT, /* 53 */
	ACCEPT, /* 54 */
	ACCEPT, /* 55 */
	ACCEPT, /* 56 */
	ACCEPT, /* 57 */
	ACCEPT, /* 58 */
	ACCEPT, /* 59 */
	ACCEPT, /* 60 */
	ACCEPT, /* 61 */
	ACCEPT, /* 62 */
	ACCEPT, /* 63 */
	ACCEPT, /* 64 */

	/* upper */
	UPPER, /* 65 */
	UPPER, /* 66 */
	UPPER, /* 67 */
	UPPER, /* 68 */
	UPPER, /* 69 */
	UPPER, /* 70 */
	UPPER, /* 71 */
	UPPER, /* 72 */
	UPPER, /* 73 */
	UPPER, /* 74 */
	UPPER, /* 75 */
	UPPER, /* 76 */
	UPPER, /* 77 */
	UPPER, /* 78 */
	UPPER, /* 79 */
	UPPER, /* 80 */
	UPPER, /* 81 */
	UPPER, /* 82 */
	UPPER, /* 83 */
	UPPER, /* 84 */
	UPPER, /* 85 */
	UPPER, /* 86 */
	UPPER, /* 87 */
	UPPER, /* 88 */
	UPPER, /* 89 */
	UPPER, /* 90 */

	ACCEPT, /* 91 */
	ACCEPT, /* 92 */
	ACCEPT, /* 93 */
	ACCEPT, /* 94 */
	ACCEPT, /* 95 */
	ACCEPT, /* 96 */

	/* lower */
	LOWER, /* 97 */
	LOWER, /* 98 */
	LOWER, /* 99 */
	LOWER, /* 100 */
	LOWER, /* 101 */
	LOWER, /* 102 */
	LOWER, /* 103 */
	LOWER, /* 104 */
	LOWER, /* 105 */
	LOWER, /* 106 */
	LOWER, /* 107 */
	LOWER, /* 108 */
	LOWER, /* 109 */
	LOWER, /* 110 */
	LOWER, /* 111 */
	LOWER, /* 112 */
	LOWER, /* 113 */
	LOWER, /* 114 */
	LOWER, /* 115 */
	LOWER, /* 116 */
	LOWER, /* 117 */
	LOWER, /* 118 */
	LOWER, /* 119 */
	LOWER, /* 120 */
	LOWER, /* 121 */
	LOWER, /* 122 */

	ACCEPT, /* 123 */
	ACCEPT, /* 124 */
	ACCEPT, /* 125 */
	ACCEPT, /* 126 */
	ACCEPT, /* 127 */

	/* del */
	REJECT, /* 128 */

	ACCEPT, /* 129 */
	ACCEPT, /* 130 */
	ACCEPT, /* 131 */
	ACCEPT, /* 132 */
	ACCEPT, /* 133 */
	ACCEPT, /* 134 */
	ACCEPT, /* 135 */
	ACCEPT, /* 136 */
	ACCEPT, /* 137 */
	ACCEPT, /* 138 */
	ACCEPT, /* 139 */
	ACCEPT, /* 140 */
	ACCEPT, /* 141 */
	ACCEPT, /* 142 */
	ACCEPT, /* 143 */
	ACCEPT, /* 144 */
	ACCEPT, /* 145 */
	ACCEPT, /* 146 */
	ACCEPT, /* 147 */
	ACCEPT, /* 148 */
	ACCEPT, /* 149 */
	ACCEPT, /* 150 */
	ACCEPT, /* 151 */
	ACCEPT, /* 152 */
	ACCEPT, /* 153 */
	ACCEPT, /* 154 */
	ACCEPT, /* 155 */
	ACCEPT, /* 156 */
	ACCEPT, /* 157 */
	ACCEPT, /* 158 */
	ACCEPT, /* 159 */
	ACCEPT, /* 160 */
	ACCEPT, /* 161 */
	ACCEPT, /* 162 */
	ACCEPT, /* 163 */
	ACCEPT, /* 164 */
	ACCEPT, /* 165 */
	ACCEPT, /* 166 */
	ACCEPT, /* 167 */
	ACCEPT, /* 168 */
	ACCEPT, /* 169 */
	ACCEPT, /* 170 */
	ACCEPT, /* 171 */
	ACCEPT, /* 172 */
	ACCEPT, /* 173 */
	ACCEPT, /* 174 */
	ACCEPT, /* 175 */
	ACCEPT, /* 176 */
	ACCEPT, /* 177 */
	ACCEPT, /* 178 */
	ACCEPT, /* 179 */
	ACCEPT, /* 180 */
	ACCEPT, /* 181 */
	ACCEPT, /* 182 */
	ACCEPT, /* 183 */
	ACCEPT, /* 184 */
	ACCEPT, /* 185 */
	ACCEPT, /* 186 */
	ACCEPT, /* 187 */
	ACCEPT, /* 188 */
	ACCEPT, /* 189 */
	ACCEPT, /* 190 */
	ACCEPT, /* 191 */
	ACCEPT, /* 192 */
	ACCEPT, /* 193 */
	ACCEPT, /* 194 */
	ACCEPT, /* 195 */
	ACCEPT, /* 196 */
	ACCEPT, /* 197 */
	ACCEPT, /* 198 */
	ACCEPT, /* 199 */
	ACCEPT, /* 200 */
	ACCEPT, /* 201 */
	ACCEPT, /* 202 */
	ACCEPT, /* 203 */
	ACCEPT, /* 204 */
	ACCEPT, /* 205 */
	ACCEPT, /* 206 */
	ACCEPT, /* 207 */
	ACCEPT, /* 208 */
	ACCEPT, /* 209 */
	ACCEPT, /* 210 */
	ACCEPT, /* 211 */
	ACCEPT, /* 212 */
	ACCEPT, /* 213 */
	ACCEPT, /* 214 */
	ACCEPT, /* 215 */
	ACCEPT, /* 216 */
	ACCEPT, /* 217 */
	ACCEPT, /* 218 */
	ACCEPT, /* 219 */
	ACCEPT, /* 220 */
	ACCEPT, /* 221 */
	ACCEPT, /* 222 */
	ACCEPT, /* 223 */
	ACCEPT, /* 224 */
	ACCEPT, /* 225 */
	ACCEPT, /* 226 */
	ACCEPT, /* 227 */
	ACCEPT, /* 228 */
	ACCEPT, /* 229 */
	ACCEPT, /* 230 */
	ACCEPT, /* 231 */
	ACCEPT, /* 232 */
	ACCEPT, /* 233 */
	ACCEPT, /* 234 */
	ACCEPT, /* 235 */
	ACCEPT, /* 236 */
	ACCEPT, /* 237 */
	ACCEPT, /* 238 */
	ACCEPT, /* 239 */
	ACCEPT, /* 240 */
	ACCEPT, /* 241 */
	ACCEPT, /* 242 */
	ACCEPT, /* 243 */
	ACCEPT, /* 244 */
	ACCEPT, /* 245 */
	ACCEPT, /* 246 */
	ACCEPT, /* 247 */
	ACCEPT, /* 248 */
	ACCEPT, /* 249 */
	ACCEPT, /* 250 */
	ACCEPT, /* 251 */
	ACCEPT, /* 252 */
	ACCEPT, /* 253 */
	ACCEPT, /* 254 */
	ACCEPT, /* 255 */

};

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

#define PRINT_LITERAL(s)                    \
	fwrite(s, 1, sizeof(s) - 1, stdout)

static INLINE void fgrep(const char *ptn, const char *filename, const size_t ptnlen, const size_t flen)
{
	FILE *fp = fopen(filename, "r");
	if (unlikely(!fp))
		return;
	g_wanted = 0;
	g_NL = 1;
	g_lnp = g_ln;
	g_lnlowerp = g_lnlower;
	filename = filename + g_fuldirlen + 1;

#define PRINT_LN(i)                                                                               \
	do {                                                                                      \
		g_lnlen = (g_lnp + i) - g_ln + 1;                                                 \
		if ((g_found = memmem(g_lnlower, g_lnlen, ptn, ptnlen))) {                        \
			g_found = g_ln + (g_found - g_lnlower);                                   \
			g_lnlowerp = g_lnlower;                                                   \
			CPY_N_ADV(g_lnlowerp, ANSI_RED);                                          \
			CPY_N_ADV_LEN(g_lnlowerp, filename, flen);                                \
			CPY_N_ADV(g_lnlowerp, ANSI_RESET ":" ANSI_GREEN);                         \
			g_NLbufp = g_NLbuf;                                                       \
			itoa_uint_pos(g_NLbufp, g_NL, 10, g_NLbufdigits);                         \
			CPY_N_ADV_LEN(g_lnlowerp, g_NLbufp, g_NLbufdigits);                       \
			CPY_N_ADV(g_lnlowerp, ANSI_RESET ":");                                    \
			flockfile(stdout);                                                        \
			fwrite(g_lnlower, 1, g_lnlowerp - g_lnlower, stdout);                     \
			fwrite(g_ln, 1, g_found - g_ln - 1, stdout);                              \
			PRINT_LITERAL(ANSI_RED);                                                  \
			fwrite(g_found, 1, ptnlen, stdout);                                       \
			PRINT_LITERAL(ANSI_RESET);                                                \
			fwrite(g_found + ptnlen, 1, g_lnlen - (g_found - g_ln + ptnlen), stdout); \
			funlockfile(stdout);                                                      \
		}                                                                                 \
	} while (0)

	do {

#define LOOP_FGREP(i)                                    \
	do {                                             \
		g_c = fgetc(fp);                         \
		switch (g_table[g_c + 1]) {              \
		case WANTED_UPPER:                       \
			g_wanted = 1;                    \
			/* FALLTHROUGH */                \
		case UPPER:                              \
			g_lnp[i] = g_c;                  \
			g_lnlowerp[i] = g_c - 'A' + 'a'; \
			break;                           \
		case WANTED:                             \
			g_wanted = 1;                    \
			/* FALLTHROUGH */                \
		default:                                 \
			g_lnp[i] = g_c;                  \
			g_lnlowerp[i] = g_c;             \
			break;                           \
		case NEWLINE:                            \
			g_lnp[i] = '\n';                 \
			if (g_wanted) {                  \
				PRINT_LN(i);             \
				g_wanted = 0;            \
			}                                \
			++g_NL;                          \
			g_lnp = g_ln;                    \
			g_lnlowerp = g_lnlower;          \
			goto CONT;                       \
		case END_OF_FILE:                        \
			g_lnp[i] = '\n';                 \
			if (g_wanted)                    \
				PRINT_LN(i);             \
			/* FALLTHROUGH */                \
		case REJECT:                             \
			goto OUT;                        \
		}                                        \
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

#define IF_DIR_RECUR_IF_REG_DO(FUNC_SELF, FUNC_REG, USE_LEN)  \
	switch (ep->d_type) {                                 \
		case DT_REG:                                  \
			FIND_FGREP_DO_REG(FUNC_REG, USE_LEN); \
			break;                                \
		case DT_DIR:                                  \
			/* skip . , .., .git, .vscode */      \
			FIND_FGREP_DO_DIR(FUNC_SELF);         \
	}                                                     \

#else

#define IF_DIR_RECUR_IF_REG_DO(FUNC_SELF, FUNC_REG, USE_LEN) \
	if (unlikely(stat(dir, &g_st)))                      \
		continue;                                    \
	if (S_ISREG(g_st.st_mode)) {                         \
		FIND_FGREP_DO_REG(FUNC_REG, USE_LEN);        \
	} else if (S_ISDIR(g_st.st_mode)) {                  \
		FIND_FGREP_DO_DIR(FUNC_SELF);                \
	}

#endif /* _DIRENT_HAVE_D_TYPE */

long g_child_max;
unsigned int g_child_tot;
pid_t pid = 1;

#define DEF_FIND_T(F, DO, USE_LEN)                                                     \
static int F(const char *ptn, const size_t ptnlen, const char *dir, const size_t dlen) \
{                                                                                      \
	DIR *dp = opendir(dir);                                                        \
	if (unlikely(!dp))                                                             \
		return 0;                                                              \
	struct dirent *ep;                                                             \
	char fulpath[MAX_PATH_LEN];                                                    \
	while ((ep = readdir(dp))) {                                                   \
		IF_DIR_RECUR_IF_REG_DO(F, DO, USE_LEN)                                 \
	}                                                                              \
	closedir(dp);                                                                  \
	return 1;                                                                      \
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
		fwrite_locked(g_ln, 1, (g_lnp + i) - g_ln + 1, stdout); \
		g_lnp = g_ln;                                           \
		CPY_N_ADV(g_lnp, ANSI_RED);                             \
		CPY_N_ADV_LEN(g_lnp, filename, flen);                   \
		CPY_N_ADV(g_lnp, ANSI_RESET ":" ANSI_GREEN);            \
		g_NLbufp = g_NLbuf;                                     \
		itoa_uint_pos(g_NLbufp, g_NL, 10, g_NLbufdigits);       \
		CPY_N_ADV_LEN(g_lnp, g_NLbufp, g_NLbufdigits);          \
		CPY_N_ADV(g_lnp, ANSI_RESET ":");                       \
		++g_NL;                                                 \
		goto CONT;                                              \
	case EOF:                                                       \
		g_lnp[i] = '\n';                                        \
		fwrite_locked(g_ln, 1, (g_lnp + i) - g_ln + 1, stdout); \
	case '\0':                                                      \
		CASE_UNPRINTABLE_WO_NUL_TAB_NL                          \
		goto OUT;                                               \
	}                                                               \

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

#define FIND_CAT_DO_REG                                                                          \
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
		if (S_ISREG(g_st.st_mode)) {
			FIND_CAT_DO_REG;
		} else if (S_ISDIR(g_st.st_mode)) {
			FIND_CAT_DO_DIR;
		}
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
	char *ptnp = ptn;
	g_lnp = PTN_;
	for (;; ++g_lnp, ++ptnp) {
		switch (*g_lnp) {
		CASE_UPPER
			*ptnp = *g_lnp - 'A' + 'a';
			continue;
		default:
			*ptnp = *g_lnp;
			continue;
		case '\0':;
		}
		break;
	}
	*ptnp = '\0';
	g_table[(unsigned char)*ptn + 1] = WANTED;
	g_table[(unsigned char)*ptn - 'a' + 'A' + 1] = WANTED_UPPER;
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
