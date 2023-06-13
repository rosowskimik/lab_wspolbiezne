#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void cleanup(void* _) {
  printf("Cleanup\n");
}

void* thread_func(void* _) {
  pthread_cleanup_push(cleanup, NULL);
  for (size_t i = 0;; ++i) {
    printf("I'm thread_func: %lu\n", i);
    // ++i;
    // pthread_testcancel();
    // sleep(1);
  }
  // pthread_cleanup_pop(0);
  return NULL;
}

int main() {
  pthread_t thread;
  void* res;

  pthread_create(&thread, NULL, thread_func, NULL);

  sleep(3);
  printf("About to cancel thread\n");

  pthread_cancel(thread);

  pthread_join(thread, &res);
  if (res == PTHREAD_CANCELED) {
    printf("Thread was canceled\n");
  } else {
    printf("Thread was not canceled\n");
  }

  return 0;
}