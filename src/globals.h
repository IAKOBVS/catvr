#ifndef GLOBALS_DEF_H
#define GLOBALS_DEF_H

#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "global_table.h"

static char g_ln[MAX_LINE_LEN];
static char g_lnlower[MAX_LINE_LEN];
static char *g_lnp;
static char *g_lnlowerp;
static char *g_NLbufp;
static char g_NLbuf[UINT_LEN];
static const char *g_found;
static unsigned int g_NL;
static unsigned int g_NLbufdigits;
static unsigned int g_fuldirlen;
static unsigned int g_lnlen;
static struct stat g_st;
static int g_c;
static int g_first_match;

static unsigned int g_child_tot;
static pid_t pid = 1;

#endif /* GLOBALS_DEF_H */
