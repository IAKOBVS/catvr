#ifndef MACROS_DEF_H
#define MACROS_DEF_H

#if defined(static_assert)
#	define HAS_STATIC_ASSERT
#	define STATIC_ASSERT(expr, msg) static_assert(expr, msg)
#elif __STDC_VERSION__ >= 201112L
#	define HAS_STATIC_ASSERT
#	define STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)
#else
#	define ASSERT(expr, msg)
#endif // static_assert

#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 24
#	if _POSIX_C_SOURCE >= 199309L
#		define HAS_GETC_UNLOCKED
#		define HAS_GETCHAR_UNLOCKED
#		define HAS_PUTC_UNLOCKED
#		define HAS_PUTCHAR_UNLOCKED
#	endif /* _POSIX_C_SOURCE */
#elif __GLIBC__ >= 2 && __GLIBC_MINOR__ <= 23
#	ifdef _POSIX_C_SOURCE
#		define HAS_GETC_UNLOCKED
#		define HAS_GETCHAR_UNLOCKED
#		define HAS_PUTC_UNLOCKED
#		define HAS_PUTCHAR_UNLOCKED
#	endif /* _POSIX_C_SOURCE */
#elif __GLIBC__ >= 2 && __GLIBC_MINOR__ <= 19
#	if defined(_SVID_SOURCE) || defined(_BSD_SOURCE)
#		define HAS_GETC_UNLOCKED
#		define HAS_GETCHAR_UNLOCKED
#		define HAS_PUTC_UNLOCKED
#		define HAS_PUTCHAR_UNLOCKED
#	endif /* _SVID_SOURCE || _BSD_SOURCE */
#endif /* GETC, GETCHAR, PUTC, PUTCHAR UNLOCKED */

#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 19
#	if _DEFAULT_SOURCE
#		define HAS_FREAD_UNLOCKED
#		define HAS_FWRITE_UNLOCKED
#		define HAS_FPUTC_UNLOCKED
#		define HAS_FGETC_UNLOCKED
#	elif defined(_SVID_SOURCE) || defined(_BSD_SOURCE)
#		define HAS_FREAD_UNLOCKED
#		define HAS_FWRITE_UNLOCKED
#		define HAS_FPUTC_UNLOCKED
#		define HAS_FGETC_UNLOCKED
#	endif /* FREAD, FWRITE, FPUTC, FGETC UNLOCKED */
#endif /* GLIBC >= 2.19 */

#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 10
#	if _POSIX_C_SOURCE >= 200809L
#		define HAS_GETLINE
#		define HAS_GETDELIM
#	endif /* _POSIX_C_SOURCE */
#else
#	ifdef _GNU_SOURCE
#		define HAS_GETLINE
#		define HAS_GETDELIM
#	endif /* _GNU_SOURCE */
#endif /* GETLINE, GETDELIM */

#ifdef _GNU_SOURCE
#	define HAS_MEMMEM
#	define HAS_STPCPY
#	define HAS_STRCASESTR
#	define HAS_FGETS_UNLOCKED
#	define HAS_MEMPCPY
#elif defined(__GLIBC__)
#	if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 10 && _POSIX_C_SOURCE >= 200809
#		define HAS_STPCPY
#	endif
#endif /* MEMMEM, STPCPY */

#if defined(__GNUC__) || defined(__clang__)
#	define INLINE __attribute__((always_inline)) inline
#	if __has_attribute(pure)
#		define PURE __attribute__((pure))
#	else
#		define PURE
#	endif /* PURE */
#	if __has_attribute(const)
#		define CONST __attribute__((const))
#	else
#		define CONST
#	endif /* CONST */
#	if __has_attribute(flatten)
#		define FLATTEN __attribute__((flatten))
#	else
#		define FLATTEN
#	endif /* FLATTEN */
#elif defined(_MSC_VER)
#	define INLINE __forceinline inline
#	define PURE   __declspec(noalias)
#	define CONST  __declspec(restrict)
#	define FLATTEN
#else
#	define INLINE inline
#	define PURE
#	define CONST
#	define FLATTEN
#endif /* __GNUC__ || __clang__ || _MSC_VER */

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || (defined(__clang__) && __has_builtin(__builtin_expect))
#	define likely(x)   __builtin_expect(!!(x), 1)
#	define unlikely(x) __builtin_expect(!!(x), 0)
#else
#	define likely(x)   (x)
#	define unlikely(x) (x)
#endif /* __has_builtin(__builtin_expect) */

#ifdef __cplusplus
#	define RESTRICT
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#	define RESTRICT restrict
#elif defined(__GNUC__) || defined(__clang__)
#	define RESTRICT __restrict__
#elif defined(_MSC_VER)
#	define RESTRICT __restrict
#else
#	define RESTRICT
#endif // restrict

#define CASE_VOWEL_LOWER \
case 'a':                \
case 'i':                \
case 'u':                \
case 'e':                \
case 'o':
#define CASE_VOWEL_UPPER \
case 'A':                \
case 'I':                \
case 'U':                \
case 'E':                \
case 'O':
#define CASE_VOWEL CASE_VOWEL_UPPER CASE_VOWEL_LOWER

#define CASE_CONSONANT_LOWER \
case 'b':                    \
case 'c':                    \
case 'd':                    \
case 'f':                    \
case 'g':                    \
case 'h':                    \
case 'j':                    \
case 'k':                    \
case 'l':                    \
case 'm':                    \
case 'n':                    \
case 'p':                    \
case 'q':                    \
case 'r':                    \
case 's':                    \
case 't':                    \
case 'v':                    \
case 'w':                    \
case 'x':                    \
case 'y':                    \
case 'z':
#define CASE_CONSONANT_UPPER \
case 'B':                    \
case 'C':                    \
case 'D':                    \
case 'F':                    \
case 'G':                    \
case 'H':                    \
case 'J':                    \
case 'K':                    \
case 'L':                    \
case 'M':                    \
case 'N':                    \
case 'P':                    \
case 'Q':                    \
case 'R':                    \
case 'S':                    \
case 'T':                    \
case 'V':                    \
case 'W':                    \
case 'X':                    \
case 'Y':                    \
case 'Z':
#define CASE_CONSONANT CASE_CONSONANT_UPPER CASE_CONSONANT_LOWER

#define CASE_DIGIT \
case '0':          \
case '1':          \
case '2':          \
case '3':          \
case '4':          \
case '5':          \
case '6':          \
case '7':          \
case '8':          \
case '9':
#define CASE_LOWER \
case 'a':          \
case 'b':          \
case 'c':          \
case 'd':          \
case 'e':          \
case 'f':          \
case 'g':          \
case 'h':          \
case 'i':          \
case 'j':          \
case 'k':          \
case 'l':          \
case 'm':          \
case 'n':          \
case 'o':          \
case 'p':          \
case 'q':          \
case 'r':          \
case 's':          \
case 't':          \
case 'u':          \
case 'v':          \
case 'w':          \
case 'x':          \
case 'y':          \
case 'z':
#define CASE_UPPER \
case 'A':          \
case 'B':          \
case 'C':          \
case 'D':          \
case 'E':          \
case 'F':          \
case 'G':          \
case 'H':          \
case 'I':          \
case 'J':          \
case 'K':          \
case 'L':          \
case 'M':          \
case 'N':          \
case 'O':          \
case 'P':          \
case 'Q':          \
case 'R':          \
case 'S':          \
case 'T':          \
case 'U':          \
case 'V':          \
case 'W':          \
case 'X':          \
case 'Y':          \
case 'Z':

#define CASE_ALPHA    CASE_LOWER CASE_UPPER
#define CASE_ALPHANUM CASE_DIGIT CASE_ALPHA

#define CASE_PRINTABLE \
case 32:               \
case 33:               \
case 34:               \
case 35:               \
case 36:               \
case 37:               \
case 38:               \
case 39:               \
case 40:               \
case 41:               \
case 42:               \
case 43:               \
case 44:               \
case 45:               \
case 46:               \
case 47:               \
case 48:               \
case 49:               \
case 50:               \
case 51:               \
case 52:               \
case 53:               \
case 54:               \
case 55:               \
case 56:               \
case 57:               \
case 58:               \
case 59:               \
case 60:               \
case 61:               \
case 62:               \
case 63:               \
case 64:               \
case 65:               \
case 66:               \
case 67:               \
case 68:               \
case 69:               \
case 70:               \
case 71:               \
case 72:               \
case 73:               \
case 74:               \
case 75:               \
case 76:               \
case 77:               \
case 78:               \
case 79:               \
case 80:               \
case 81:               \
case 82:               \
case 83:               \
case 84:               \
case 85:               \
case 86:               \
case 87:               \
case 88:               \
case 89:               \
case 90:               \
case 91:               \
case 92:               \
case 93:               \
case 94:               \
case 95:               \
case 96:               \
case 97:               \
case 98:               \
case 99:               \
case 100:              \
case 101:              \
case 102:              \
case 103:              \
case 104:              \
case 105:              \
case 106:              \
case 107:              \
case 108:              \
case 109:              \
case 110:              \
case 111:              \
case 112:              \
case 113:              \
case 114:              \
case 115:              \
case 116:              \
case 117:              \
case 118:              \
case 119:              \
case 120:              \
case 121:              \
case 122:              \
case 123:              \
case 124:              \
case 125:              \
case 126:

#define CASE_UNPRINTABLE_WO_NUL_TAB_NL \
case 1:                                \
case 2:                                \
case 3:                                \
case 4:                                \
case 5:                                \
case 6:                                \
case 7:                                \
case 8:                                \
case 11:                               \
case 12:                               \
case 13:                               \
case 14:                               \
case 15:                               \
case 16:                               \
case 17:                               \
case 18:                               \
case 19:                               \
case 20:                               \
case 21:                               \
case 22:                               \
case 23:                               \
case 24:                               \
case 25:                               \
case 26:                               \
case 27:                               \
case 28:                               \
case 29:                               \
case 30:                               \
case 31:                               \
case 127:

#endif /* MACROS_DEF_H */
