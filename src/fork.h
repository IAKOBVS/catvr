#ifndef FORK_DEF_H
#define FORK_DEF_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
/* #include <semaphore.h> */

#include "config.h"
#include "librgrep.h"

#define g_child_max MAX_FORKS

static pid_t g_pid = 1;
static int *g_child_alive;
/* static sem_t g_alive_mutex; */

static INLINE void init_shm()
{
	char buf[64] = "/tmp/rgrep";
	char num[UINT_LEN];
	char *nump = num;
	int dgts;
	itoa_uint_pos(nump, getpid(), 10, dgts);
	memcpy(buf + sizeof("/tmp/rgrep") - 1, nump, dgts);
	buf[dgts] = '\0';
	key_t key = ftok(buf, 0);
	dgts = shmget(key, sizeof(int), 0666 | IPC_CREAT);
	g_child_alive = shmat(dgts, NULL, 0);
	*g_child_alive = 0;
	/* sem_init(&g_alive_mutex, 1, 1); */
}

static INLINE void free_shm()
{
	/* sem_destroy(&g_alive_mutex); */
	while (wait(NULL) != -1)
		;
	shmdt(&g_child_alive);
}

#define IF_FORK_MAX_WAIT_CHILD                               \
	do {                                                 \
		if (unlikely(*g_child_alive >= g_child_max)) \
			wait(NULL);                          \
	} while (0)

#define FORK_AND_WAIT(DO)                               \
	do {                                            \
		if (unlikely(g_pid == 0)) {             \
			DO;                             \
		} else {                                \
			IF_FORK_MAX_WAIT_CHILD;         \
			fflush(stdout);                 \
			g_pid = fork();                 \
			switch (g_pid) {                \
			case 0:                         \
				++*g_child_alive;       \
				DO;                     \
				--*g_child_alive;       \
				_exit(0);               \
				break;                  \
			default:                        \
				IF_FORK_MAX_WAIT_CHILD; \
			case -1:;                       \
			}                               \
		}                                       \
	} while (0)

#endif /* FORK_DEF_H */\n