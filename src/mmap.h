#ifndef MMAP_DEF_H
#define MMAP_DEF_H

#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
/* #include "macros.h" */
/* #include <stddef.h> */

/* void *mmap_open(const char *RESTRICT filename, size_t *RESTRICT filesz, int *RESTRICT fd); */
/* void mmap_close(void *RESTRICT p, const char *RESTRICT filename, size_t filesz, int fd); */

#ifdef _DIRENT_HAVE_D_TYPE
	/* if d_type is not available, it already calls stat in find */
#	define STAT_IF_EMPTY                      \
		if (unlikely(stat(filename, &st))) \
		return
#else
#	define STAT_IF_EMPTY
#endif /* _DIRENT_HAVE_D_TYPE */

#define MALLOC_OPEN(buf, filename, filesz)                           \
	do {                                                         \
		FILE *fp = fopen(filename, "r");                     \
		if (unlikely(!fp))                                   \
			return;                                      \
		STAT_IF_EMPTY;                                       \
		if (unlikely(!st.st_size))                           \
			return;                                      \
		if (unlikely(st.st_size > MAX_FILE_SZ))              \
			return;                                      \
		if ((size_t)st.st_size > g_bufsz) {                  \
			free(buf);                                   \
			g_bufsz = st.st_size;                        \
			buf = malloc(st.st_size);                    \
			if (unlikely(!buf)) {                        \
				fgrep_err("Can't malloc", filename); \
				exit(1);                             \
			}                                            \
		}                                                    \
		filesz = st.st_size;                                 \
		fread(buf, 1, filesz, fp);                           \
		fclose(fp);                                          \
	} while (0)

#define MALLOC_CLOSE(buf)

#endif /* MMAP_DEF_H */
