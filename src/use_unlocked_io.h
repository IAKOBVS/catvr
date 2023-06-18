#ifndef UNLOCKED_MACROS_DEF_H
#define UNLOCKED_MACROS_DEF_H

#ifdef HAS_FGETC_UNLOCKED
#	define fgetc(c) fgetc_unlocked(c)
#endif

#ifdef HAS_GETC_UNLOCKED
#	define getc(fp) getc_unlocked(fp)
#endif

#ifdef HAS_PUTCHAR_UNLOCKED
#	define putchar(c) putchar_unlocked(c)
#endif

#ifdef HAS_FGETS_UNLOCKED
#	define fgets(s, N, fp) fgets_unlocked(s, N, fp)
#endif

#ifdef HAS_FWRITE_UNLOCKED
#	define fwrite(s, sz, N, fp)	    fwrite_unlocked(s, sz, N, fp)
#	define fwrite_locked(s, sz, N, fp) fwrite(s, sz, N, fp)
#else
#	define fwrite_locked(s, sz, N, fp) fwrite(s, sz, N, fp)
#	define flockfile(fp)
#	define funlockfile(fp)
#endif

#ifndef HAS_MEMMEM
#	define memmem(haystack, haystacklen, needle, needlelen) strstr(haystack, needle)
#endif /* !HAS_MEMMEM */

#endif /* UNLOCKED_MACROS_DEF_H */