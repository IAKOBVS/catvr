#include <stdio.h>
#include <assert.h>

#define CASE_PRINTABLE case 9: case 32: case 33: case 34: case 35: case 36: case 37: case 38: case 39: case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47: case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 58: case 59: case 60: case 61: case 62: case 63: case 64: case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 91: case 92: case 93: case 94: case 95: case 96: case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: case 123: case 124: case 125: case 126:  

static inline void catv(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (!fp)
		return;
	char ln[4096];
	while (fgets(ln, 4096, fp)) {
		for (char *lp = ln;; ++lp) {
			switch (*lp) {
			CASE_PRINTABLE
				continue;
			default:;
			}
			break;
		}
		printf("%s", ln);
	}
	fclose(fp);
}

int main(int argc, char **argv)
{
	assert(argv[1]);
	catv(argv[1]);
	return 0;
}
