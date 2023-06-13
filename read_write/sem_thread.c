#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
// #include <unistd.h>
#include <stdalign.h>

#define BUFSIZE 7
#define LOOPS 40
#define IDX(idx) (idx % BUFSIZE)

#define READERS 2
#define WRITERS 2

char buffer[BUFSIZE];
sem_t read_s, write_s;
alignas(4096) atomic_size_t read_idx = 0, write_idx = 0;

void* writer(void* arg) {
  size_t start = (size_t)arg;
  for (size_t i = 0; i < LOOPS; ++i) {
    sem_wait(&write_s);
    buffer[IDX(atomic_fetch_add_explicit(&write_idx, 1,
                                         memory_order_relaxed))] = start + i;
    sem_post(&read_s);
  }
  return 0;
}

void* reader() {
  for (size_t i = 0; i < LOOPS; ++i) {
    sem_wait(&read_s);
    printf("%d\n", buffer[IDX(atomic_fetch_add_explicit(
                       &read_idx, 1, memory_order_relaxed))]);
    sem_post(&write_s);
  }
  return 0;
}

int main() {
  sem_init(&read_s, 0, 0);
  sem_init(&write_s, 0, BUFSIZE);

  pthread_t read_h[READERS], write_h[WRITERS];
  for (size_t i = 0; i < WRITERS; ++i) {
    pthread_create(&write_h[i], NULL, writer, i * LOOPS);
  }
  for (size_t i = 0; i < READERS; ++i) {
    pthread_create(&read_h[i], NULL, reader, NULL);
  }

  for (size_t i = 0; i < READERS; ++i) {
    pthread_join(read_h[i], NULL);
  }
  for (size_t i = 0; i < WRITERS; ++i) {
    pthread_join(write_h[i], NULL);
  }

  sem_destroy(&read_s);
  sem_destroy(&write_s);
  return 0;
}