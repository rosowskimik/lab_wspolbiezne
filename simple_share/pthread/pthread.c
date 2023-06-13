#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#define LOOP_COUNT 300000
#define THREAD_COUNT 100

int c1 = 0;
atomic_int c2 = 0;

void* inc(void* _) {
  for (int i = 0; i < LOOP_COUNT; ++i) {
    // Non atomic
    int t1 = c1;
    t1 = t1 + 1;
    c1 = t1;

    // Atomic
    int tmp;
    for (;;) {
      int curr = atomic_load_explicit(&c2, memory_order_relaxed);
      tmp = curr + 1;
      if (atomic_compare_exchange_weak_explicit(
              &c2, &curr, tmp, memory_order_acq_rel, memory_order_relaxed)) {
        break;
      }
    }
  }
  return 0;
}

int main() {
  pthread_t ths[THREAD_COUNT];
  for (int i = 0; i < THREAD_COUNT; ++i) {
    pthread_create(&ths[i], NULL, inc, NULL);
  }

  for (int i = 0; i < THREAD_COUNT; ++i) {
    pthread_join(ths[i], NULL);
  }

  printf("c1 = %d, c2 = %d\n", c1, c2);
}
