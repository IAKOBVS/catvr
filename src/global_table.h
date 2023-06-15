#ifndef GLOBAL_TABLE_DEF_H
#define GLOBAL_TABLE_DEF_H

enum { ACCEPT = 0,
	REJECT,
	NEWLINE,
	UPPER,
	LOWER,
	WANTED,
	WANTED_UPPER,
	FILLED,
	END_OF_FILE };

static unsigned char g_table[257] = {

	/* EOF */
	END_OF_FILE, /* -1 */

	/* unprintable */
	REJECT, /* 0 */
	REJECT, /* 1 */
	REJECT, /* 2 */
	REJECT, /* 3 */
	REJECT, /* 4 */
	REJECT, /* 5 */
	REJECT, /* 6 */
	REJECT, /* 7 */
	REJECT, /* 8 */

	/* tab and newline */
	ACCEPT, /* 9 */
	NEWLINE, /* 10 */

	REJECT, /* 11 */
	REJECT, /* 12 */
	REJECT, /* 13 */
	REJECT, /* 14 */
	REJECT, /* 15 */
	REJECT, /* 16 */
	REJECT, /* 17 */
	REJECT, /* 18 */
	REJECT, /* 19 */
	REJECT, /* 20 */
	REJECT, /* 21 */
	REJECT, /* 22 */
	REJECT, /* 23 */
	REJECT, /* 24 */
	REJECT, /* 25 */
	REJECT, /* 26 */
	REJECT, /* 27 */
	REJECT, /* 28 */
	REJECT, /* 29 */
	REJECT, /* 30 */
	REJECT, /* 31 */

	/* printable */
	ACCEPT, /* 32 */
	ACCEPT, /* 33 */
	ACCEPT, /* 34 */
	ACCEPT, /* 35 */
	ACCEPT, /* 36 */
	ACCEPT, /* 37 */
	ACCEPT, /* 38 */
	ACCEPT, /* 39 */
	ACCEPT, /* 40 */
	ACCEPT, /* 41 */
	ACCEPT, /* 42 */
	ACCEPT, /* 43 */
	ACCEPT, /* 44 */
	ACCEPT, /* 45 */
	ACCEPT, /* 46 */
	ACCEPT, /* 47 */
	ACCEPT, /* 48 */
	ACCEPT, /* 49 */
	ACCEPT, /* 50 */
	ACCEPT, /* 51 */
	ACCEPT, /* 52 */
	ACCEPT, /* 53 */
	ACCEPT, /* 54 */
	ACCEPT, /* 55 */
	ACCEPT, /* 56 */
	ACCEPT, /* 57 */
	ACCEPT, /* 58 */
	ACCEPT, /* 59 */
	ACCEPT, /* 60 */
	ACCEPT, /* 61 */
	ACCEPT, /* 62 */
	ACCEPT, /* 63 */
	ACCEPT, /* 64 */

	/* upper */
	UPPER, /* 65 */
	UPPER, /* 66 */
	UPPER, /* 67 */
	UPPER, /* 68 */
	UPPER, /* 69 */
	UPPER, /* 70 */
	UPPER, /* 71 */
	UPPER, /* 72 */
	UPPER, /* 73 */
	UPPER, /* 74 */
	UPPER, /* 75 */
	UPPER, /* 76 */
	UPPER, /* 77 */
	UPPER, /* 78 */
	UPPER, /* 79 */
	UPPER, /* 80 */
	UPPER, /* 81 */
	UPPER, /* 82 */
	UPPER, /* 83 */
	UPPER, /* 84 */
	UPPER, /* 85 */
	UPPER, /* 86 */
	UPPER, /* 87 */
	UPPER, /* 88 */
	UPPER, /* 89 */
	UPPER, /* 90 */

	ACCEPT, /* 91 */
	ACCEPT, /* 92 */
	ACCEPT, /* 93 */
	ACCEPT, /* 94 */
	ACCEPT, /* 95 */
	ACCEPT, /* 96 */

	/* lower */
	LOWER, /* 97 */
	LOWER, /* 98 */
	LOWER, /* 99 */
	LOWER, /* 100 */
	LOWER, /* 101 */
	LOWER, /* 102 */
	LOWER, /* 103 */
	LOWER, /* 104 */
	LOWER, /* 105 */
	LOWER, /* 106 */
	LOWER, /* 107 */
	LOWER, /* 108 */
	LOWER, /* 109 */
	LOWER, /* 110 */
	LOWER, /* 111 */
	LOWER, /* 112 */
	LOWER, /* 113 */
	LOWER, /* 114 */
	LOWER, /* 115 */
	LOWER, /* 116 */
	LOWER, /* 117 */
	LOWER, /* 118 */
	LOWER, /* 119 */
	LOWER, /* 120 */
	LOWER, /* 121 */
	LOWER, /* 122 */

	ACCEPT, /* 123 */
	ACCEPT, /* 124 */
	ACCEPT, /* 125 */
	ACCEPT, /* 126 */
	ACCEPT, /* 127 */

	/* del */
	REJECT, /* 128 */

	ACCEPT, /* 129 */
	ACCEPT, /* 130 */
	ACCEPT, /* 131 */
	ACCEPT, /* 132 */
	ACCEPT, /* 133 */
	ACCEPT, /* 134 */
	ACCEPT, /* 135 */
	ACCEPT, /* 136 */
	ACCEPT, /* 137 */
	ACCEPT, /* 138 */
	ACCEPT, /* 139 */
	ACCEPT, /* 140 */
	ACCEPT, /* 141 */
	ACCEPT, /* 142 */
	ACCEPT, /* 143 */
	ACCEPT, /* 144 */
	ACCEPT, /* 145 */
	ACCEPT, /* 146 */
	ACCEPT, /* 147 */
	ACCEPT, /* 148 */
	ACCEPT, /* 149 */
	ACCEPT, /* 150 */
	ACCEPT, /* 151 */
	ACCEPT, /* 152 */
	ACCEPT, /* 153 */
	ACCEPT, /* 154 */
	ACCEPT, /* 155 */
	ACCEPT, /* 156 */
	ACCEPT, /* 157 */
	ACCEPT, /* 158 */
	ACCEPT, /* 159 */
	ACCEPT, /* 160 */
	ACCEPT, /* 161 */
	ACCEPT, /* 162 */
	ACCEPT, /* 163 */
	ACCEPT, /* 164 */
	ACCEPT, /* 165 */
	ACCEPT, /* 166 */
	ACCEPT, /* 167 */
	ACCEPT, /* 168 */
	ACCEPT, /* 169 */
	ACCEPT, /* 170 */
	ACCEPT, /* 171 */
	ACCEPT, /* 172 */
	ACCEPT, /* 173 */
	ACCEPT, /* 174 */
	ACCEPT, /* 175 */
	ACCEPT, /* 176 */
	ACCEPT, /* 177 */
	ACCEPT, /* 178 */
	ACCEPT, /* 179 */
	ACCEPT, /* 180 */
	ACCEPT, /* 181 */
	ACCEPT, /* 182 */
	ACCEPT, /* 183 */
	ACCEPT, /* 184 */
	ACCEPT, /* 185 */
	ACCEPT, /* 186 */
	ACCEPT, /* 187 */
	ACCEPT, /* 188 */
	ACCEPT, /* 189 */
	ACCEPT, /* 190 */
	ACCEPT, /* 191 */
	ACCEPT, /* 192 */
	ACCEPT, /* 193 */
	ACCEPT, /* 194 */
	ACCEPT, /* 195 */
	ACCEPT, /* 196 */
	ACCEPT, /* 197 */
	ACCEPT, /* 198 */
	ACCEPT, /* 199 */
	ACCEPT, /* 200 */
	ACCEPT, /* 201 */
	ACCEPT, /* 202 */
	ACCEPT, /* 203 */
	ACCEPT, /* 204 */
	ACCEPT, /* 205 */
	ACCEPT, /* 206 */
	ACCEPT, /* 207 */
	ACCEPT, /* 208 */
	ACCEPT, /* 209 */
	ACCEPT, /* 210 */
	ACCEPT, /* 211 */
	ACCEPT, /* 212 */
	ACCEPT, /* 213 */
	ACCEPT, /* 214 */
	ACCEPT, /* 215 */
	ACCEPT, /* 216 */
	ACCEPT, /* 217 */
	ACCEPT, /* 218 */
	ACCEPT, /* 219 */
	ACCEPT, /* 220 */
	ACCEPT, /* 221 */
	ACCEPT, /* 222 */
	ACCEPT, /* 223 */
	ACCEPT, /* 224 */
	ACCEPT, /* 225 */
	ACCEPT, /* 226 */
	ACCEPT, /* 227 */
	ACCEPT, /* 228 */
	ACCEPT, /* 229 */
	ACCEPT, /* 230 */
	ACCEPT, /* 231 */
	ACCEPT, /* 232 */
	ACCEPT, /* 233 */
	ACCEPT, /* 234 */
	ACCEPT, /* 235 */
	ACCEPT, /* 236 */
	ACCEPT, /* 237 */
	ACCEPT, /* 238 */
	ACCEPT, /* 239 */
	ACCEPT, /* 240 */
	ACCEPT, /* 241 */
	ACCEPT, /* 242 */
	ACCEPT, /* 243 */
	ACCEPT, /* 244 */
	ACCEPT, /* 245 */
	ACCEPT, /* 246 */
	ACCEPT, /* 247 */
	ACCEPT, /* 248 */
	ACCEPT, /* 249 */
	ACCEPT, /* 250 */
	ACCEPT, /* 251 */
	ACCEPT, /* 252 */
	ACCEPT, /* 253 */
	ACCEPT, /* 254 */
	ACCEPT, /* 255 */

};

#endif /* GLOBAL_TABLE_DEF_H */