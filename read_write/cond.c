#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 7
#define LOOPS 50
#define IDX(idx) (idx % BUFSIZE)

#define READERS 5
#define WRITERS 2

static int buffer[BUFSIZE];
static size_t buf_count = 0;
static pthread_mutex_t buflock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t bufcond = PTHREAD_COND_INITIALIZER;

void* reader(void* arg) {
  printf("\e[92mReader started\n");
  for (size_t i = 0; i < LOOPS; ++i) {
    pthread_mutex_lock(&buflock);
    printf("\e[92mReader lock\n");
    while (!buf_count) {
      printf("\e[92mBuffer empty, waiting for more...\n");
      pthread_cond_signal(&bufcond);
      pthread_cond_wait(&bufcond, &buflock);
      printf("\e[92mMore values in buffer\n");
    }
    printf("\e[37m(Buf entries: %lu) Value at %lu: %d\n",
           (*(volatile size_t*)&buf_count), i, buffer[IDX(i)]);
    --buf_count;
    pthread_cond_signal(&bufcond);
    pthread_mutex_unlock(&buflock);
    // if (rand() % 5 < 2) {
    //   sched_yield();
    // }
  }

  return NULL;
}

void* writer(void* arg) {
  printf("\e[91mWriter started\n");
  for (size_t i = 0; i < LOOPS; ++i) {
    pthread_mutex_lock(&buflock);
    printf("\e[91mWriter lock\n");
    while (buf_count == BUFSIZE) {
      printf("\e[91mBuffer full, waiting for space...\n");
      pthread_cond_signal(&bufcond);
      pthread_cond_wait(&bufcond, &buflock);
      printf("\e[91mFree space\n");
    }

    printf("\e[37m(Buf entries: %lu) Writing value %d at %lu\n", buf_count,
           (int)i, i);
    buffer[IDX(i)] = i;
    ++buf_count;
    pthread_cond_signal(&bufcond);
    pthread_mutex_unlock(&buflock);
  }

  return NULL;
}

int main() {
  pthread_t read_h[READERS], write_h[WRITERS];
  for (size_t i = 0; i < READERS; ++i) {
    pthread_create(&read_h[i], NULL, reader, NULL);
  }
  for (size_t i = 0; i < WRITERS; ++i) {
    pthread_create(&write_h[i], NULL, writer, NULL);
  }

  for (size_t i = 0; i < READERS; ++i) {
    pthread_join(read_h[i], NULL);
  }
  for (size_t i = 0; i < WRITERS; ++i) {
    pthread_join(write_h[i], NULL);
  }

  return 0;
}
