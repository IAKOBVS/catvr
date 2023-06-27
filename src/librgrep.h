#ifndef LIBGREP_DEF_H
#define LIBGREP_DEF_H

#include "macros.h"
#include <stdio.h>
#include <string.h>

#if USE_ANSI_COLORS
#	define ANSI_RED     "\x1b[31m"
#	define ANSI_GREEN   "\x1b[32m"
#	define ANSI_YELLOW  "\x1b[33m"
#	define ANSI_BLUE    "\x1b[34m"
#	define ANSI_MAGENTA "\x1b[35m"
#	define ANSI_CYAN    "\x1b[36m"
#	define ANSI_RESET   "\x1b[0m"
#else
#	define ANSI_RED     ""
#	define ANSI_GREEN   ""
#	define ANSI_YELLOW  ""
#	define ANSI_BLUE    ""
#	define ANSI_MAGENTA ""
#	define ANSI_CYAN    ""
#	define ANSI_RESET   ""
#endif /* USE_ANSI_COLORS */

static INLINE void append(char *path, const char *dir, size_t dlen, const char *filename);
static INLINE char *appendp(char *path, const char *dir, size_t dlen, const char *filename);
static INLINE void append_len(char *RESTRICT path, const char *RESTRICT dir, size_t dlen, const char *RESTRICT filename, size_t flen);

static INLINE void append(char *RESTRICT path, const char *RESTRICT dir, size_t dlen, const char *RESTRICT filename)
{
	memcpy(path, dir, dlen);
	*(path + dlen) = '/';
	memcpy(path + dlen + 1, filename, dlen + 1);
}

static INLINE char *appendp(char *RESTRICT path, const char *RESTRICT dir, size_t dlen, const char *RESTRICT filename)
{
#if defined(HAS_STPCPY) && defined(HAS_MEMPCPY)
	*(path = (char *)mempcpy(path, dir, dlen)) = '/';
	return stpcpy(path + 1, filename);
#else
	memcpy(path, dir, dlen);
	*(path + dlen) = '/';
	dlen = strlen(filename);
	return (char *)memcpy(path + dlen + 1, filename, dlen + 1) + dlen;
#endif /* HAS_STPCPY */
}

static INLINE void append_len(char *RESTRICT path, const char *RESTRICT dir, size_t dlen, const char *RESTRICT filename, size_t flen)
{
	memcpy(path, dir, dlen);
	*(path + dlen) = '/';
	memcpy(path + dlen + 1, filename, flen + 1);
}

/* Does not nul terminate */
#define itoa_uint_pos(s, n, base, digits)               \
	do {                                            \
		unsigned int n__ = n;                   \
		char *const end__ = (s) + UINT_LEN - 1; \
		(s) = end__;                            \
		do                                      \
			*(s)-- = (n__) % (base) + '0';  \
		while ((n__) /= 10);                    \
		digits = end__ - (s)++;                 \
	} while (0)

#define PRINT_LITERAL(s) fwrite((s), 1, sizeof(s) - 1, stdout)

#define IF_EXCLUDED_EXT_GOTO(filename, flen, action)                 \
	do {                                                         \
		if (flen > 1) {                                      \
			/* ignore .o files */                        \
			if (((filename)[(flen)-2]) == '.') {         \
				if (((filename)[(flen)-1]) == 'o') { \
					action;                      \
				}                                    \
			}                                            \
		}                                                    \
	} while (0)

/*
   ignored files:
   .
   ..
   .clang-format
   .eslintrc
   .editorconfig
   .jshintrc
   .gitignore
   .ignore
*/

#define IF_EXCLUDED_REG_GOTO(filename, action)                           \
	do {                                                             \
		if ((filename)[0] == '.') {                              \
			switch ((filename)[1]) {                         \
			case '.':                                        \
				/* .. */                                 \
				action;                                  \
				break;                                   \
			case '\0':                                       \
				/* . */                                  \
				action;                                  \
				break;                                   \
			case 'e':                                        \
				switch ((filename)[2]) {                 \
				/* .eslintrc */                          \
				case 's':                                \
					if (((filename)[3] == 'l')       \
					    && ((filename)[4] == 'i')    \
					    && ((filename)[5] == 'n')    \
					    && ((filename)[6] == 't')    \
					    && ((filename)[7] == 'r')    \
					    && ((filename)[8] == 'c'))   \
						action;                  \
					break;                           \
					/* .editorconfig */              \
				case 'd':                                \
					if (((filename)[3] == 'i')       \
					    && ((filename)[4] == 't')    \
					    && ((filename)[5] == 'o')    \
					    && ((filename)[6] == 'r')    \
					    && ((filename)[7] == 'c')    \
					    && ((filename)[8] == 'o')    \
					    && ((filename)[9] == 'n')    \
					    && ((filename)[10] == 'f')   \
					    && ((filename)[11] == 'i')   \
					    && ((filename)[12] == 'g'))  \
						action;                  \
				}                                        \
				break;                                   \
			case 'j':                                        \
				/* .jshintrc */                          \
				if (((filename)[2] == 's')               \
				    && ((filename)[3] == 'h')            \
				    && ((filename)[4] == 'i')            \
				    && ((filename)[5] == 'n')            \
				    && ((filename)[6] == 't')            \
				    && ((filename)[7] == 'r')            \
				    && ((filename)[8] == 'c'))           \
					action;                          \
				break;                                   \
			case 'g':                                        \
				if ((filename)[2] == 'i') {              \
					/* .gitignore */                 \
					if (((filename)[3] == 't')       \
					    && ((filename)[4] == 'i')    \
					    && ((filename)[5] == 'g')    \
					    && ((filename)[6] == 'n')    \
					    && ((filename)[7] == 'o')    \
					    && ((filename)[8] == 'r')    \
					    && ((filename)[9] == 'e')) { \
						action;                  \
					}                                \
				}                                        \
				break;                                   \
			case 'c':                                        \
				/* .clang-format */                      \
				if ((filename)[2] == 'l'                 \
				    && ((filename)[3] == 'a')            \
				    && ((filename)[4] == 'n')            \
				    && ((filename)[5] == 'g')            \
				    && ((filename)[6] == '-')            \
				    && ((filename)[7] == 'f')            \
				    && ((filename)[8] == 'o')            \
				    && ((filename)[9] == 'r')            \
				    && ((filename)[10] == 'm')           \
				    && ((filename)[11] == 'a')           \
				    && ((filename)[12] == 't')) {        \
					action;                          \
				}                                        \
				break;                                   \
				/* .ignore */                            \
			case 'i':                                        \
				if (((filename)[2] == 'g')               \
				    && ((filename)[3] == 'n')            \
				    && ((filename)[4] == 'o')            \
				    && ((filename)[5] == 'r')            \
				    && ((filename)[6] == 'e')) {         \
					action;                          \
				}                                        \
			}                                                \
		}                                                        \
	} while (0)

/*
   ignored directories:
   .
   ..
   .git
   .vscode
*/

#define IF_EXCLUDED_DIR_GOTO(filename, action)                 \
	do {                                                   \
		if ((filename)[0] == '.')                      \
			switch ((filename)[1]) {               \
			case '.':                              \
				/* .. */                       \
				action;                        \
				break;                         \
				/* . */                        \
			case '\0':                             \
				action;                        \
				break;                         \
				/* .git */                     \
			case 'g':                              \
				if ((filename)[2] == 'i'       \
				    && (filename)[3] == 't') { \
					action;                \
				}                              \
				break;                         \
				/* .vscode */                  \
			case 'v':                              \
				if ((filename)[2] == 's'       \
				    && (filename)[3] == 'c'    \
				    && (filename)[4] == 'o'    \
				    && (filename)[5] == 'd'    \
				    && (filename)[6] == 'e') { \
					action;                \
				}                              \
				break;                         \
			}                                      \
	} while (0)

#endif /* LIBGREP_DEF_H */
