#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/stat.h>

#include "../lib/libcatvr.h"

#define DEBUG 0
#define PRINT_PER_CHAR 1

enum {
	MAX_LINE_LEN = 4096,
	MAX_PATH_LEN = 4096,
};

int g_NL;
size_t g_fuldirlen;
struct stat g_st;

#if PRINT_PER_CHAR
char g_c;
#else
char g_ln[MAX_LINE_LEN];
#endif /* PRINT_PER_CHAR */

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
#endif

#if PRINT_PER_CHAR
static INLINE void catv(const char *RESTRICT filename, const size_t flen)
{
	FILE *RESTRICT fp = fopen(filename, "r");
	if (unlikely(!fp))
		return;
	g_NL = 0;
	fwrite(ANSI_RED ":" ANSI_GREEN "0" ANSI_RESET ":", 1, sizeof(ANSI_RED ":" ANSI_GREEN "0" ANSI_RESET ":") - 1, stdout);
	fwrite(filename + g_fuldirlen + 1, 1, flen, stdout);
	for (;;) {
		switch (g_c = getc(fp)) {
		default:
		case '\t':
			putchar(g_c);
			continue;
		case '\n':
			fwrite("\n" ANSI_RED, 1, sizeof("\n" ANSI_RED) - 1, stdout);
			fwrite(filename + g_fuldirlen + 1, 1, flen, stdout);
			fwrite(&g_NL, sizeof(int), 1, stdout);
			continue;
		case '\0':
		CASE_UNPRINTABLE_WO_NUL_TAB_NL
		case EOF:
			putchar('\n');
		}
		break;
	}
	fclose(fp);
}

#else
static INLINE void catv(const char *RESTRICT filename)
{
	FILE *RESTRICT fp = fopen(filename, "r");
	if (unlikely(!fp))
		return;
#if DEBUG
	printf("filename: %s\n", filename);
#endif /* DEBUG */
	g_NL = 0;
	while (fgets(g_ln, MAX_LINE_LEN, fp)) {
		++g_NL;
		for (char *RESTRICT lp = g_ln;; ++lp) {
			switch (*lp) {
			default:
				continue;
			case '\0':
			CASE_UNPRINTABLE_WO_NUL_TAB_NL
				goto OUT;
			case '\n':;
			}
			break;
		}
		printf(ANSI_RED "%s" ANSI_RESET ":" ANSI_GREEN "%d" ANSI_RESET ":%s\n", filename + g_fuldirlen + 1, g_NL, g_ln);
	}
OUT:
	fclose(fp);
}
#endif

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

static void findall(const char *RESTRICT dir, const size_t dlen)
{
	DIR *RESTRICT dp = opendir(dir);
	if (unlikely(!dp))
		return;
	struct dirent *RESTRICT ep;
	char fulpath[MAX_PATH_LEN];
	while ((ep = readdir(dp))) {
#if DEBUG
		puts(ep->d_name);
#endif /* DEBUG */
#ifdef _DIRENT_HAVE_D_TYPE
		switch (ep->d_type) {
		case DT_REG:
			catv(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath);
			break;
		case DT_DIR:
			/* skip . , .., .git, .vscode */
			IF_EXCLUDED_DO(ep->d_name, continue)
			findall(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath);
		}
#else
		if (unlikely(stat(dir, &g_st)))
			return;
		if (S_ISREG(g_st.st_mode)) {
			catv(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - (fulpath + dlen));
		} else if (S_ISDIR(g_st.st_mode)) {
			IF_EXCLUDED_DO(ep->d_name, continue)
			findall(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - (fulpath + dlen));
		}
#endif /* _DIRENT_HAVE_D_TYPE */
#if DEBUG
		printf("entries: %s\n", ep->d_name);
#endif /* DEBUG */
	}
	closedir(dp);
}

#define DIRECTORY argv[1]

int main(int argc, char **argv)
{
	if (unlikely(argc == 1))
		goto GET_CWD;
	switch (DIRECTORY[0]) {
	case '.':
		if (unlikely(DIRECTORY[1] == '\0'))
			goto GET_CWD;
		/* FALLTHROUGH */
	default:
		if (unlikely(stat(DIRECTORY, &g_st))) {
			printf("%s not a valid file or dir\n", DIRECTORY);
			return 1;
		}
		if (unlikely(S_ISREG(g_st.st_mode))) {
			g_fuldirlen = strrchr(DIRECTORY, '/') - DIRECTORY;
			catv(DIRECTORY, g_fuldirlen);
		} else {
			g_fuldirlen = strlen(DIRECTORY);
			findall(DIRECTORY, g_fuldirlen);
		}
		break;
	case '\0':
	GET_CWD:;
		char cwd[MAX_PATH_LEN];
		getcwd(cwd, MAX_PATH_LEN);
		g_fuldirlen = strlen(cwd);
		findall(cwd, g_fuldirlen);
		break;
	}
	return 0;
}
