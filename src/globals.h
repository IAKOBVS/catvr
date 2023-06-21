#ifndef GLOBALS_DEF_H

#	include <stddef.h>
#	include <sys/stat.h>

static unsigned char *g_buf;
static size_t g_bufsz;
static struct stat st;

#	define GLOBALS_DEF_H
#endif /* GLOBALS_DEF_H */
