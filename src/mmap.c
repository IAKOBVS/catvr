#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "macros.h"
#include "librgrep.h"

void *mmap_open(const char *RESTRICT filename, size_t *RESTRICT filesz, int *RESTRICT fd)
{
	*fd = open(filename, O_RDONLY, S_IRUSR);
	if (unlikely(*fd < 0)) {
		fgrep_err("Negative *fd", filename);
		exit(1);
	}
	struct stat st;
	if (unlikely(fstat(*fd, &st))) {
		fgrep_err("Can't fstat", filename);
		exit(1);
	}
	*filesz = st.st_size;
	return mmap(NULL, *filesz, PROT_READ, MAP_PRIVATE, *fd, 0);
}

void mmap_close(void *RESTRICT p, const char *RESTRICT filename, size_t filesz, int fd)
{
	if (unlikely(close(fd))) {
		fgrep_err("Can't close", filename);
		exit(1);
	}
	if (unlikely(munmap(p, filesz))) {
		fgrep_err("Can't munmap", filename);
		exit(1);
	}
}