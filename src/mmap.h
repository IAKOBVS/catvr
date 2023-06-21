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

#include <fcntl.h>
#include <unistd.h>

#include "globals.h"

/* #include "macros.h" */
/* #include <stddef.h> */

/* void *mmap_open(const char *RESTRICT filename, size_t *RESTRICT filesz, int *RESTRICT fd); */
/* void mmap_close(void *RESTRICT p, const char *RESTRICT filename, size_t filesz, int fd); */

#ifdef _DIRENT_HAVE_D_TYPE
/* if d_type is not available, it already calls stat in find */
#	define CHECK_FILESZ(fd) fstat(fd, &st)
#else
#	define STAT_IF_EMPTY(fd)
#endif /* _DIRENT_HAVE_D_TYPE */

#define MALLOC_OPEN(filename, filesz)                                \
	do {                                                         \
		int fd = open(filename, O_RDONLY);                   \
		if (unlikely(fd < 0))                                \
			return;                                      \
		CHECK_FILESZ(fd);                                    \
		if (unlikely(!st.st_size))                           \
			return;                                      \
		if (unlikely(st.st_size > MAX_FILE_SZ))              \
			return;                                      \
		if ((size_t)st.st_size > g_bufsz) {                  \
			free(g_buf);                                 \
			g_buf = malloc(st.st_size);                  \
			if (unlikely(!g_buf)) {                      \
				fgrep_err("Can't malloc", filename); \
				exit(1);                             \
			}                                            \
			g_bufsz = st.st_size;                        \
		}                                                    \
		filesz = st.st_size;                                 \
		read(fd, g_buf, filesz);                             \
		close(fd);                                           \
	} while (0)

#endif /* MMAP_DEF_H */
