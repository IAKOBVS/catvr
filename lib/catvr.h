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
#endif /* GLIBC >= 2.24 */

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
#	endif /* _DEFAULT_SOURCE */
#endif /* GLIBC >= 2.19 */

#ifdef _GNU_SOURCE
#	define HAS_MEMMEM
#	define HAS_STPCPY
#	define HAS_STRCASESTR
#	define HAS_FGETS_UNLOCKED
#elif defined(__GLIBC__)
#	if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 10 && _POSIX_C_SOURCE >= 200809
#		define HAS_STPCPY
#	endif
#endif /* HAS_MEMMEM, HAS_STPCPY */

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
#	define PURE __declspec(noalias)
#	define CONST __declspec(restrict)
#	define FLATTEN
#else
#	define INLINE inline
#	define PURE
#	define CONST
#	define FLATTEN
#endif /* __GNUC__ || __clang__ || _MSC_VER */

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || (defined(__clang__) && __has_builtin(__builtin_expect))
#	define likely(x) __builtin_expect(!!(x), 1)
#	define unlikely(x) __builtin_expect(!!(x), 0)
#else
#	define likely(x) (x)
#	define unlikely(x) (x)
#endif /* __has_builtin(__builtin_expect) */

#define CASE_PRINTABLE \
	case 32:       \
	case 33:       \
	case 34:       \
	case 35:       \
	case 36:       \
	case 37:       \
	case 38:       \
	case 39:       \
	case 40:       \
	case 41:       \
	case 42:       \
	case 43:       \
	case 44:       \
	case 45:       \
	case 46:       \
	case 47:       \
	case 48:       \
	case 49:       \
	case 50:       \
	case 51:       \
	case 52:       \
	case 53:       \
	case 54:       \
	case 55:       \
	case 56:       \
	case 57:       \
	case 58:       \
	case 59:       \
	case 60:       \
	case 61:       \
	case 62:       \
	case 63:       \
	case 64:       \
	case 65:       \
	case 66:       \
	case 67:       \
	case 68:       \
	case 69:       \
	case 70:       \
	case 71:       \
	case 72:       \
	case 73:       \
	case 74:       \
	case 75:       \
	case 76:       \
	case 77:       \
	case 78:       \
	case 79:       \
	case 80:       \
	case 81:       \
	case 82:       \
	case 83:       \
	case 84:       \
	case 85:       \
	case 86:       \
	case 87:       \
	case 88:       \
	case 89:       \
	case 90:       \
	case 91:       \
	case 92:       \
	case 93:       \
	case 94:       \
	case 95:       \
	case 96:       \
	case 97:       \
	case 98:       \
	case 99:       \
	case 100:      \
	case 101:      \
	case 102:      \
	case 103:      \
	case 104:      \
	case 105:      \
	case 106:      \
	case 107:      \
	case 108:      \
	case 109:      \
	case 110:      \
	case 111:      \
	case 112:      \
	case 113:      \
	case 114:      \
	case 115:      \
	case 116:      \
	case 117:      \
	case 118:      \
	case 119:      \
	case 120:      \
	case 121:      \
	case 122:      \
	case 123:      \
	case 124:      \
	case 125:      \
	case 126:
