#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>

#define LOOP_COUNT 1000000
#define THREAD_COUNT 4

atomic_flag lock = ATOMIC_FLAG_INIT;
unsigned long c1, c2;

void spin_lock() {
  while (atomic_flag_test_and_set_explicit(&lock, memory_order_acquire)) {
    continue;
  }
}

void spin_unlock() {
  atomic_flag_clear_explicit(&lock, memory_order_release);
}

void* inc() {
  for (int i = 0; i < LOOP_COUNT; i++) {
    spin_lock();
    int tmp = 0;
    sched_yield();
    tmp = c1 + 1;
    sched_yield();
    c1 = tmp;
    spin_unlock();

    sched_yield();
    tmp = c2 + 1;
    sched_yield();
    c2 = tmp;
  }
  return NULL;
}

int main() {
  pthread_t ths[THREAD_COUNT];

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_create(&ths[i], NULL, inc, NULL);
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(ths[i], NULL);
  }

  printf("c1 = %lu, c2 = %lu\n", c1, c2);
  return 0;
}
