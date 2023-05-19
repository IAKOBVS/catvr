#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _GNU_SOURCE
#	define HAS_MEMMEM
#	define HAS_STPCPY
#	define HAS_STRCASESTR
#elif defined(__GLIBC__)
#	if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1 && _POSIX_C_SOURCE >= 200809
#		define HAS_STPCPY
#	endif
#endif /* HAS_MEMMEM, HAS_STPCPY */

#if defined(__GNUC__) || defined(__clang__)
#	define INLINE __attribute__((always_inline)) inline
#	if __has_attribute(pure)
#		define PURE __attribute__((pure))
#	else
#		define PURE
#	endif /* PURE */
#	if __has_attribute(const)
#		define CONST __attribute__((const))
#	else
#		define CONST
#	endif /* CONST */
#	if __has_attribute(flatten)
#		define FLATTEN __attribute__((flatten))
#	else
#		define FLATTEN
#	endif /* FLATTEN */
#elif defined(_MSC_VER)
#	define INLINE __forceinline inline
#	define PURE __declspec(noalias)
#	define CONST __declspec(restrict)
#	define FLATTEN
#else
#	define INLINE inline
#	define PURE
#	define CONST
#	define FLATTEN
#endif /* __GNUC__ || __clang__ || _MSC_VER */

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || (defined(__clang__) && __has_builtin(__builtin_expect))
#	define likely(x) __builtin_expect(!!(x), 1)
#	define unlikely(x) __builtin_expect(!!(x), 0)
#else
#	define likely(x) (x)
#	define unlikely(x) (x)
#endif /* __has_builtin(__builtin_expect) */

#define CASE_PRINTABLE \
	case 32:       \
	case 33:       \
	case 34:       \
	case 35:       \
	case 36:       \
	case 37:       \
	case 38:       \
	case 39:       \
	case 40:       \
	case 41:       \
	case 42:       \
	case 43:       \
	case 44:       \
	case 45:       \
	case 46:       \
	case 47:       \
	case 48:       \
	case 49:       \
	case 50:       \
	case 51:       \
	case 52:       \
	case 53:       \
	case 54:       \
	case 55:       \
	case 56:       \
	case 57:       \
	case 58:       \
	case 59:       \
	case 60:       \
	case 61:       \
	case 62:       \
	case 63:       \
	case 64:       \
	case 65:       \
	case 66:       \
	case 67:       \
	case 68:       \
	case 69:       \
	case 70:       \
	case 71:       \
	case 72:       \
	case 73:       \
	case 74:       \
	case 75:       \
	case 76:       \
	case 77:       \
	case 78:       \
	case 79:       \
	case 80:       \
	case 81:       \
	case 82:       \
	case 83:       \
	case 84:       \
	case 85:       \
	case 86:       \
	case 87:       \
	case 88:       \
	case 89:       \
	case 90:       \
	case 91:       \
	case 92:       \
	case 93:       \
	case 94:       \
	case 95:       \
	case 96:       \
	case 97:       \
	case 98:       \
	case 99:       \
	case 100:      \
	case 101:      \
	case 102:      \
	case 103:      \
	case 104:      \
	case 105:      \
	case 106:      \
	case 107:      \
	case 108:      \
	case 109:      \
	case 110:      \
	case 111:      \
	case 112:      \
	case 113:      \
	case 114:      \
	case 115:      \
	case 116:      \
	case 117:      \
	case 118:      \
	case 119:      \
	case 120:      \
	case 121:      \
	case 122:      \
	case 123:      \
	case 124:      \
	case 125:      \
	case 126:

enum {
	MAX_LINE_LEN = 4096,
	MAX_PATH_LEN = 4096,
};

static INLINE void append(char *fulpath, const char *dname, size_t dlen, const char *filename)
{
	memcpy(fulpath, dname, dlen);
	*(fulpath += dlen) = '/';
	memcpy(++fulpath, filename, dlen + 1);
}

static INLINE char *appendp(char *fulpath, const char *dname, size_t dlen, const char *filename)
{
	memcpy(fulpath, dname, dlen);
	*(fulpath += dlen) = '/';
#ifdef HAS_STPCPY
	return stpcpy(++fulpath, filename);
#else
	dlen = strlen(filename);
	return (char *)memcpy(++fulpath, filename, dlen + 1) + dlen;
#endif /* HAS_STPCPY */
}

static INLINE void catv(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (unlikely(!fp))
		return;
	printf("filename: %s\n", filename);
	char ln[4096];
	int LN = 0;
	while (fgets(ln, 4096, fp)) {
		++LN;
		for (char *lp = ln;; ++lp) {
			switch (*lp) {
			CASE_PRINTABLE
			case '\t':
				continue;
			default:
				goto OUT;
			case '\n':;
			}
			break;
		}
		printf("%s:%d:%s", filename, LN, ln);
	}
OUT:
	fclose(fp);
}

#define IF_EXCLUDED_DO(filename, action)               \
	if (filename[0] == '.')                        \
		switch (filename[1]) {                 \
		case '.':                              \
		case '\0':                             \
			action;                        \
		case 'i':                              \
			if (filename[2] == 't')        \
				action;                \
			break;                         \
		case 'v':                              \
			if (filename[2] == 's'         \
				&& filename[3] == 'c'  \
				&& filename[4] == 'o'  \
				&& filename[5] == 'd'  \
				&& filename[6] == 'e') \
				action;                \
			break;                         \
		}

#define DEBUG 1

static int findall(const char *dname, const size_t dlen)
{
	DIR *dp = opendir(dname);
	if (unlikely(!dp))
		return 0;
	struct dirent *ep;
	char fulpath[MAX_PATH_LEN];
#ifndef _DIRENT_HAVE_D_TYPE
	struct stat st;
#endif /* _DIRENT_HAVE_D_TYPE */
	while ((ep = readdir(dp))) {
#ifdef _DIRENT_HAVE_D_TYPE
		switch (ep->d_type) {
		case DT_REG:
			append(fulpath, dname, dlen, ep->d_name);
			catv(fulpath);
			break;
		case DT_DIR:
			/* skip . , .., .git, .vscode */
			IF_EXCLUDED_DO(ep->d_name, continue)
			findall(fulpath, appendp(fulpath, dname, dlen, ep->d_name) - fulpath);
		}
#else
		if (unlikely(stat(dname, &st)))
			return 0;
		if (S_ISREG(st.st_mode)) {
			append(fulpath, dname, dlen, ep->d_name);
			catv(fulpath);
		} else if (S_ISDIR(st.st_mode)) {
			IF_EXCLUDED_DO(ep->d_name, continue)
			findall(fulpath, appendp(fulpath, dname, dlen, ep->d_name) - fulpath);
		}
#endif /* _DIRENT_HAVE_D_TYPE */
#ifdef DEBUG
		printf("dir: %s\n", ep->d_name);
#endif /* DEBUG */
	}
	closedir(dp);
	return 1;
}

int main(int argc, char **argv)
{
	char *dname = argv[1];
	size_t dlen;
	if (!argv[1]) {
		dname = ".";
		dlen = 1;
	} else
		dlen = strlen(dname);
	findall(dname, dlen);
	return 0;
}
