#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define LOOP_COUNT 150000000
#define THREAD_COUNT 100

_Thread_local unsigned long counter = 0;
unsigned long global_counter = 0;
static unsigned long* counterp[THREAD_COUNT] = {NULL};
static pthread_mutex_t counterp_lock = PTHREAD_MUTEX_INITIALIZER;

void register_thread(size_t i) {
  pthread_mutex_lock(&counterp_lock);
  counterp[i] = &counter;
  pthread_mutex_unlock(&counterp_lock);
}

void unregister_thread(size_t i) {
  pthread_mutex_lock(&counterp_lock);
  global_counter += counter;
  counterp[i] = NULL;
  pthread_mutex_unlock(&counterp_lock);
}

void* inc(void* arg) {
  register_thread((size_t)arg);
  for (int i = 0; i < LOOP_COUNT; ++i) {
    ++counter;
  }
  unregister_thread((size_t)arg);
  return 0;
}

unsigned long read_counter() {
  pthread_mutex_lock(&counterp_lock);
  unsigned long sum = global_counter;
  for (int i = 0; i < THREAD_COUNT; ++i) {
    if (counterp[i] != NULL) {
      sum += *counterp[i];
    }
  }
  pthread_mutex_unlock(&counterp_lock);
  return sum;
}

int main() {
  pthread_t ths[THREAD_COUNT];

  for (size_t i = 0; i < THREAD_COUNT; ++i) {
    pthread_create(&ths[i], NULL, inc, (void*)i);
  }

  for (size_t i = 0; i < THREAD_COUNT; ++i) {
    pthread_join(ths[i], NULL);
  }

  printf("read_counter = %lu\n", read_counter());
}
