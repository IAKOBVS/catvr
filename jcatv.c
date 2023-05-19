#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "jcatv.h"

enum {
	DEBUG = 0,
	MAX_LINE_LEN = 4096,
	MAX_PATH_LEN = 4096,
};

char ln[MAX_LINE_LEN];
int NL;
size_t fuldirlen;
char *fuldir;

static INLINE void append(char *path, const char *dir, size_t dlen, const char *filename)
{
	memcpy(path, dir, dlen);
	*(path += dlen) = '/';
	memcpy(++path, filename, dlen + 1);
}

static INLINE char *appendp(char *path, const char *dir, size_t dlen, const char *filename)
{
	memcpy(path, dir, dlen);
	*(path += dlen) = '/';
#ifdef HAS_STPCPY
	return stpcpy(++path, filename);
#else
	dlen = strlen(filename);
	return (char *)memcpy(++path, filename, dlen + 1) + dlen;
#endif /* HAS_STPCPY */
}

static INLINE void catv(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (unlikely(!fp))
		return;
	NL = 0;
#if DEBUG == 1
	printf("filename: %s\n", filename);
#endif /* DEBUG */
#ifdef HAS_FGETS_UNLOCKED
	while (fgets_unlocked(ln, MAX_LINE_LEN, fp)) {
#else
	while (fgets(ln, MAX_LINE_LEN, fp)) {
#endif /* _GNU_SOURCE */
		++NL;
		for (char *lp = ln;; ++lp) {
			switch (*lp) {
				CASE_PRINTABLE
			case '\t': continue;
			default: goto OUT;
			case '\n':;
			}
			break;
		}
		printf("%s:%d:%s", filename + fuldirlen + 1, NL, ln);
	}
OUT:
	fclose(fp);
}

/* skip . , .., .git, .vscode */
#define IF_EXCLUDED_DO(filename, action)               \
	if (filename[0] == '.')                        \
		switch (filename[1]) {                 \
		case '.':                              \
		case '\0':                             \
			action;                        \
		case 'g':                              \
			if (filename[2] == 'i'         \
				&& filename[3] == 't') \
				action;                \
			break;                         \
		case 'v':                              \
			if (filename[2] == 's'         \
				&& filename[3] == 'c'  \
				&& filename[4] == 'o'  \
				&& filename[5] == 'd'  \
				&& filename[6] == 'e') \
				action;                \
		}

static int findall(const char *dir, const size_t dlen)
{
	DIR *dp = opendir(dir);
	if (unlikely(!dp))
		return 0;
	struct dirent *ep;
	char fulpath[MAX_PATH_LEN];
#ifndef _DIRENT_HAVE_D_TYPE
	struct stat st;
#endif /* _DIRENT_HAVE_D_TYPE */
	while ((ep = readdir(dp))) {
#if DEBUG == 1
		puts(ep->d_name);
#endif /* DEBUG */
#ifdef _DIRENT_HAVE_D_TYPE
		switch (ep->d_type) {
		case DT_REG:
			append(fulpath, dir, dlen, ep->d_name);
			catv(fulpath);
			break;
		case DT_DIR:
			/* skip . , .., .git, .vscode */
			IF_EXCLUDED_DO(ep->d_name, continue)
			findall(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath);
		}
#else
		if (unlikely(stat(dir, &st)))
			return 0;
		if (S_ISREG(st.st_mode)) {
			append(fulpath, dir, dlen, ep->d_name);
			catv(fulpath);
		} else if (S_ISDIR(st.st_mode)) {
			IF_EXCLUDED_DO(ep->d_name, continue)
			findall(fulpath, appendp(fulpath, dir, dlen, ep->d_name) - fulpath);
		}
#endif /* _DIRENT_HAVE_D_TYPE */
#if DEBUG == 1
		printf("entries: %s\n", ep->d_name);
#endif /* DEBUG */
	}
	closedir(dp);
	return 1;
}

int main(int argc, char **argv)
{
	if (!(fuldir = argv[1])) {
		char cwd[MAX_PATH_LEN];
		fuldir = getcwd(cwd, MAX_PATH_LEN);
		fuldirlen = strlen(fuldir);
		findall(fuldir, fuldirlen);
	} else {
		fuldirlen = strlen(fuldir);
		findall(fuldir, fuldirlen);
	}
	return 0;
}
