#ifndef MMAP_DEF_H
#define MMAP_DEF_H

#if defined(__GNUC__) || defined(__GLIBC__)
#	ifndef _GNU_SOURCE
#		define _GNU_SOURCE
#	endif
#endif /* _GNU_SOURCE */

#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "globals.h"

#ifdef _DIRENT_HAVE_D_TYPE
/* if d_type is not available, it already calls stat in find */
#	define CHECK_FILESZ(fd) fstat(fd, &st)
#else
#	define STAT_IF_EMPTY(fd)
#endif /* _DIRENT_HAVE_D_TYPE */

#define PROG_NAME "rgrep"

#define MALLOC_DIE(p, filename)                                                    \
	do {                                                                       \
		if (unlikely(!p)) {                                                \
			fprintf(stderr, PROG_NAME ":Can't malloc:%s\n", filename); \
			exit(1);                                                   \
		}                                                                  \
	} while (0)

#define MALLOC_OPEN(filename, filesz)                                    \
	do {                                                             \
		int fd = open(filename, O_RDONLY);                       \
		if (unlikely(fd == -1))                                  \
			return;                                          \
		CHECK_FILESZ(fd);                                        \
		if (unlikely(!st.st_size))                               \
			return;                                          \
		if (unlikely(st.st_size > MAX_FILE_SZ))                  \
			return;                                          \
		if ((size_t)st.st_size > g_bufsz) {                      \
			do {                                             \
				if ((size_t)st.st_size < g_bufsz * 2) {  \
					g_bufsz *= 2;                    \
					break;                           \
				}                                        \
				if ((size_t)st.st_size < g_bufsz * 4) {  \
					g_bufsz *= 4;                    \
					break;                           \
				}                                        \
				if ((size_t)st.st_size < g_bufsz * 16) { \
					g_bufsz *= 16;                   \
					break;                           \
				}                                        \
				g_bufsz *= 32;                           \
			} while ((size_t)st.st_size < g_bufsz);          \
			free(g_buf);                                     \
			g_buf = malloc(st.st_size);                      \
			MALLOC_DIE(g_buf, filename);                     \
			g_bufsz = st.st_size;                            \
		}                                                        \
		filesz = st.st_size;                                     \
		read(fd, g_buf, filesz);                                 \
		close(fd);                                               \
	} while (0)

#endif /* MMAP_DEF_H */
