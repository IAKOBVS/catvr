#ifndef MMAP_DEF_H
#define MMAP_DEF_H

#include <stdlib.h>
#include <sys/stat.h>
/* #include "macros.h" */
/* #include <stddef.h> */

/* void *mmap_open(const char *RESTRICT filename, size_t *RESTRICT filesz, int *RESTRICT fd); */
/* void mmap_close(void *RESTRICT p, const char *RESTRICT filename, size_t filesz, int fd); */

#ifdef _DIRENT_HAVE_D_TYPE
#	define STAT_IF_NO_STAT                     \
		if (unlikely(stat(filename, &st))) \
			return
#else
#	define STAT_IF_NO_STAT
#endif /* _DIRENT_HAVE_D_TYPE */

#define MALLOC_OPEN(buf, filename, filesz)                                      \
	do {                                                                    \
		FILE *fp = fopen(filename, "r");                                \
		if (unlikely(!fp))                                              \
			return;                                                 \
		STAT_IF_NO_STAT;                                                \
		if (unlikely(!st.st_size))                                      \
			return;                                                 \
		if (unlikely(st.st_size > MAX_FILE_SZ))                         \
			return;                                                 \
		if ((size_t)st.st_size > g_bufsz) {                             \
			free(buf);                                              \
			buf = malloc(st.st_size);                               \
			if (unlikely(!buf)) {                                   \
				fgrep_err("Can't allocate malloc\n", filename); \
				exit(1);                                        \
			}                                                       \
			g_bufsz = st.st_size;                                   \
		}                                                               \
		filesz = st.st_size;                                            \
		if (unlikely(!buf)) {                                           \
			fgrep_err("Can't allocate malloc\n", filename);         \
			exit(1);                                                \
		}                                                               \
		fread(buf, 1, filesz, fp);                                      \
		fclose(fp);                                                     \
	} while (0)

#define MALLOC_CLOSE(buf)

#endif /* MMAP_DEF_H */
