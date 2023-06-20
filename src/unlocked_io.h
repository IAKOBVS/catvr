#ifndef UNLOCKED_IO_DEF_H
#define UNLOCKED_IO_DEF_H

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
#else
#	define flockfile(fp)
#	define funlockfile(fp)
#endif

#endif /* UNLOCKED_IO_DEF_H */
