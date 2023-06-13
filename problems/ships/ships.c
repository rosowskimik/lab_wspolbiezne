#define _DEFAULT_SOURCE

#include <linux/futex.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

int rsleep(unsigned int max_millis) {
  unsigned int target = rand() % (max_millis + 1);
  return usleep(target * 1000);
}

static int futex(uint32_t* uaddr,
                 int futex_op,
                 uint32_t val,
                 const struct timespec* timeout,
                 uint32_t* uaddr2,
                 uint32_t val3) {
  return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

typedef struct semaphore {
  _Atomic(uint32_t) counter;
} semaphore_t;

void semaphore_init(semaphore_t* s, uint32_t initial_value) {
  atomic_store_explicit(&s->counter, initial_value, memory_order_release);
}

void semaphore_wait_n(semaphore_t* s, uint32_t n) {
  uint32_t counter;
  do {
    counter = atomic_load_explicit(&s->counter, memory_order_acquire);
    if (counter < n) {
      futex((uint32_t*)&s->counter, FUTEX_WAIT, counter, NULL, NULL, 0);
      continue;
    }
  } while (!atomic_compare_exchange_weak_explicit(
      &s->counter, &counter, counter - n, memory_order_acq_rel,
      memory_order_relaxed));
}

void semaphore_post_n(semaphore_t* s, uint32_t n) {
  atomic_fetch_add_explicit(&s->counter, n, memory_order_release);
  futex((uint32_t*)&s->counter, FUTEX_WAKE, UINT32_MAX, NULL, NULL, 0);
}

#ifndef SHIP_CNT
#define SHIP_CNT 100
#endif

#ifndef MAX_PER_SHIP
#define MAX_PER_SHIP 5
#endif

#ifndef HAULER_CNT
#define HAULER_CNT 15
#endif

#if HAULER_CNT < MAX_PER_SHIP
#error \
    "HAULER_CNT is smaller than MAX_PER_SHIP - some ships won't be able to enter"
#endif

#ifndef PORT_SIZE
#define PORT_SIZE 100
#endif

#if PORT_SIZE < MAX_PER_SHIP
#error \
    "PORT_SIZE is smaller than MAX_PER_SHIP - some ships won't be able to enter"
#endif

semaphore_t port_space, haulers;

void* ship(void* arg) {
  size_t id = (size_t)arg + 1;
  unsigned int ship_size = (rand() % MAX_PER_SHIP) + 1;

  srand(time(NULL) + id);
  for (;;) {
    rsleep(10000);
    printf("Ship %03lu (size %u) wants to enter\n", id, ship_size);
    semaphore_wait_n(&port_space, ship_size);
    printf("Ship %03lu (size %u) reserved the space, waiting for haulers\n", id,
           ship_size);
    semaphore_wait_n(&haulers, ship_size);
    printf("Ship %03lu (size %u) entering port now\n", id, ship_size);
    rsleep(5000);
    printf("Ship %03lu (size %u) entered the port\n", id, ship_size);
    semaphore_post_n(&haulers, ship_size);
    rsleep(15000);
    printf("Ship %03lu (size %u) exiting the port\n", id, ship_size);
    semaphore_post_n(&port_space, ship_size);
  }
}

int main() {
  semaphore_init(&port_space, PORT_SIZE);
  semaphore_init(&haulers, HAULER_CNT);

  pthread_t ths[SHIP_CNT];
  for (size_t i = 0; i < SHIP_CNT; ++i) {
    pthread_create(&ths[i], NULL, ship, (void*)i);
  }

  for (size_t i = 0; i < SHIP_CNT; ++i) {
    pthread_join(ths[i], NULL);
  }

  // UNREACHABLE
  return 1;
}
