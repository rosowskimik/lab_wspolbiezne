#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef CART_SPACE
#define CART_SPACE 10
#endif

#ifndef PEOPLE
#define PEOPLE 100
#endif

#define handle_error(msg)           \
	do {                        \
		perror(msg);        \
		exit(EXIT_FAILURE); \
	} while (0)

int rsleep(unsigned int max_millis)
{
	unsigned int target = rand() % (max_millis + 1);
	return usleep(target * 1000);
}

int sem_multiwait(sem_t *sem, size_t cnt)
{
	for (; cnt > 0; --cnt) {
		if (sem_wait(sem) == -1)
			return -1;
	}
	return 0;
}

int sem_multipost(sem_t *sem, size_t cnt)
{
	for (; cnt > 0; --cnt) {
		if (sem_post(sem) == -1)
			return -1;
	}
	return 0;
}

typedef struct share {
	sem_t wait_room;
	sem_t cart_space;
	sem_t cart_finish;
	sem_t cart_exit;
} share_t;

share_t *share_init()
{
	const char *share_name = "/rollercoaster";

	int fd = shm_open(share_name, O_CREAT | O_TRUNC | O_RDWR, 0600);
	if (fd == -1)
		handle_error("shm_open");

	if (ftruncate(fd, sizeof(share_t)) == -1)
		handle_error("ftruncate");

	share_t *shmp = mmap(NULL, sizeof(share_t), PROT_READ | PROT_WRITE,
			     MAP_SHARED, fd, 0);
	if (shmp == MAP_FAILED)
		handle_error("mmap");

	if (shm_unlink(share_name) == -1)
		handle_error("shm_unlink");

	if (sem_init(&shmp->wait_room, 1, 0) == -1)
		handle_error("sem_init-wait_room");
	if (sem_init(&shmp->cart_space, 1, CART_SPACE) == -1)
		handle_error("sem_init-cart_space");
	if (sem_init(&shmp->cart_finish, 1, 0) == -1)
		handle_error("sem_init-cart_finish");
	if (sem_init(&shmp->cart_exit, 1, 0) == -1)
		handle_error("sem_init-cart_exit");

	return shmp;
}

static void cart_exit_handler(int sig)
{
	exit(EXIT_SUCCESS);
}

void cart_loop(share_t *s)
{
	srand(time(NULL));
	for (;;) {
		printf("Cart waiting for people\n");
		if (sem_multiwait(&s->wait_room, CART_SPACE) == -1)
			handle_error("Cart wait_room multiwait\n");

		printf("Cart full - Blastoff!\n");
		rsleep(7000);

		printf("Cart finished a ride. Waiting for people to leave\n");
		if (sem_multipost(&s->cart_finish, CART_SPACE) == -1)
			handle_error("Cart cart_finish multipost\n");

		if (sem_multiwait(&s->cart_exit, CART_SPACE) == -1)
			handle_error("Cart cart_exit multiwait\n");
		printf("Cart emptied\n");

		if (sem_multipost(&s->cart_space, CART_SPACE) == -1)
			handle_error("Cart cart_space multipost\n");
	}
}

void person(share_t *s, size_t i)
{
	srand(time(NULL) + i);
	rsleep(25000);

	if (sem_post(&s->wait_room) == -1)
		handle_error("Person wait_room\n");
	printf("Person %03lu entered waiting area\n", i);

	rsleep(2500);
	if (sem_wait(&s->cart_space) == -1)
		handle_error("Person cart_space\n");

	if (sem_wait(&s->cart_finish) == -1)
		handle_error("Person cart_finish\n");

	rsleep(2500);
	if (sem_post(&s->cart_exit) == -1)
		handle_error("Person cart_exit\n");
	printf("Person %03lu exited the cart. Goodbye\n", i);

	exit(EXIT_SUCCESS);
}

int main()
{
	share_t *s = share_init();

	// Spawn the cart
	pid_t cart_pid = fork();
	switch (cart_pid) {
	case -1:
		handle_error("Cart fork");
	case 0:
		// Setup the signal handler
		{
			struct sigaction sa;

			sa.sa_handler = cart_exit_handler;
			sigemptyset(&sa.sa_mask);
			if (sigaction(SIGTERM, &sa, NULL) == -1)
				handle_error("Cart sigaction");

			cart_loop(s);
		}
	}

	// Spawn people
	for (size_t i = 0; i < PEOPLE; ++i) {
		switch (fork()) {
		case -1:
			handle_error("Person fork");
		case 0:
			person(s, i);
		}
	}

	int result;
	for (size_t i = 0; i < PEOPLE; ++i) {
		wait(&result);
		if (!WIFEXITED(result) || WEXITSTATUS(result) == EXIT_FAILURE)
			handle_error("Person exit");
	}

	// All people processess stopped - signal cart to terminate
	if (kill(cart_pid, SIGTERM) == -1)
		handle_error("Kill");

	wait(&result);
	if (WEXITSTATUS(result) == EXIT_FAILURE)
		handle_error("Cart exit");

	printf("Everybody finished - Goodbye!\n");
	return 0;
}
