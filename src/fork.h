#ifndef FORK_DEF_H
#define FORK_DEF_H

#include "config.h"

#define g_child_max MAX_FORKS

#define IF_FORK_MAX_WAIT_CHILD                      \
	do {                                        \
		if (g_child_alive >= g_child_max) { \
			wait(NULL);                 \
			--g_child_alive;            \
		}                                   \
	} while (0)

#define FORK_AND_WAIT(DO)                               \
	do {                                            \
		if (likely(pid == 0)) {                 \
			DO;                             \
		} else {                                \
			IF_FORK_MAX_WAIT_CHILD;         \
			fflush(stdout);                 \
			pid = fork();                   \
			switch (pid) {                  \
			case 0:                         \
				DO;                     \
				_exit(0);               \
				break;                  \
			default:                        \
				++g_child_alive;        \
				IF_FORK_MAX_WAIT_CHILD; \
			case -1:;                       \
			}                               \
		}                                       \
	} while (0)

#endif /* FORK_DEF_H */
