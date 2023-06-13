#ifndef FORK_DEF_H
#define FORK_DEF_H

#define g_child_max FORK_MAX

#define IF_FORK_MAX_WAIT_CHILD                    \
	do {                                      \
		if (g_child_tot == g_child_max) { \
			wait(NULL);               \
			--g_child_tot;            \
		}                                 \
	} while (0)

#define FORK_AND_WAIT(DO)                       \
	do {                                    \
		if (pid > 0) {                  \
			IF_FORK_MAX_WAIT_CHILD; \
			pid = fork();           \
		}                               \
		switch (pid) {                  \
		case 0:                         \
			DO;                     \
			break;                  \
		default:                        \
			++g_child_tot;          \
			IF_FORK_MAX_WAIT_CHILD; \
		case -1:;                       \
		}                               \
	} while (0)

#endif /* FORK_DEF_H */
