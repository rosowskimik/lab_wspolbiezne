#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int x = 0;
sem_t rx, tx;

void* reader(void*) {
  while (1) {
    sem_wait(&rx);
    printf("Reader: %d\n", x);
    sem_post(&tx);
  }
  return 0;
}

int main() {
  pthread_t reader_thread;

  pthread_create(&reader_thread, NULL, reader, NULL);

  sem_init(&rx, 0, 0);
  sem_init(&tx, 0, 1);

  for (int i = 0; i < 10; ++i) {
    sem_wait(&tx);
    ++x;
    printf("Writer: %d\n", x);
    sem_post(&rx);
  }

  sem_wait(&tx);
  pthread_cancel(reader_thread);
  sem_destroy(&rx);
  sem_destroy(&tx);

  return 0;
}