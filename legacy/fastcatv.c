/* segfaults sometimes */

/* static INLINE void print_lines(const char *filename) */
/* { */
/* 	FILE *fp = fopen(filename, "r"); */
/* 	if (unlikely(!fp)) */
/* 		return; */
/* 	NL = 0; */
/* 	while (fgets(fbuf, MAX_LINE_LEN, fp)) { */
/* 		++NL; */
/* 		for (char *lp = fbuf;; ++lp) { */
/* 			switch (*lp) { */
/* 			CASE_PRINTABLE */
/* 			case '\t': */
/* 				continue; */
/* 			default: */
/* 				goto OUT; */
/* 			case '\n':; */
/* 			} */
/* 			break; */
/* 		} */
/* 		printf("%s:%d:%s", filename, NL, fbuf); */
/* 	} */
/* OUT: */
/* 	fclose(fp); */
/* } */

/* static INLINE void catv_f(const char *filename) */
/* { */
/* 	if (unlikely(stat(filename, &fst))) */
/* 		return; */
/* 	const size_t file_size = fst.st_size; */
/* 	if (unlikely(!file_size)) */
/* 		return; */
/* 	if (unlikely(file_size > MAX_BUF_SZ)) */
/* 		print_lines(filename); */
/* 	FILE *fp = fopen(filename, "r"); */
/* 	if (unlikely(!fp)) */
/* 		return; */
/* 	fread(fbuf, 1, file_size, fp); */
/* 	*(fbuf + file_size) = '\0'; */
/* 	if (strlen(fbuf) == file_size) { */
/* 		NL = 1; */
/* 		printf("%s:%d", filename, NL); */
/* 		for (char *p = fbuf;; ++p) { */
/* 			switch (*p) { */
/* 			case '\n': */
/* 				printf("\n%s:%d:", filename, ++NL); */
/* 				continue; */
/* 			default: */
/* 				putchar(*p); */
/* 				continue; */
/* 			case '\0':; */
/* 			} */
/* 			break; */
/* 		} */
/* 	} */
/* 	fclose(fp); */
/* } */
