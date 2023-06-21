#ifndef GLOBALS_DEF_H

#include "config.h"

#	include <stddef.h>
#	include <sys/stat.h>

static unsigned char *g_buf;
static size_t g_bufsz = MIN_BUF_SZ;
static struct stat st;

#	define GLOBALS_DEF_H
#endif /* GLOBALS_DEF_H */
