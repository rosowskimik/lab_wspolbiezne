#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

struct shared_data {
  sem_t read;
  sem_t write;
  sem_t end;

  int curr;
};

int main() {
  int fd, local_x;
  struct shared_data* shared_data;

  if ((fd = shm_open("/myshm", O_RDWR | O_CREAT | O_TRUNC, 0666)) == -1) {
    perror("shm_open");
    exit(1);
  }

  if (ftruncate(fd, sizeof(struct shared_data)) == -1) {
    perror("ftruncate");
    exit(1);
  }

  shared_data =
      (struct shared_data*)mmap(NULL, sizeof(struct shared_data),
                                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (shared_data == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  sem_init(&shared_data->read, 1, 0);
  sem_init(&shared_data->write, 1, 1);

  switch (fork()) {
    case -1:
      perror("fork");
      exit(1);
    case 0:
      // child

      for (int i = 1; i <= 100; ++i) {
        sem_wait(&shared_data->read);
        printf("Child shared read: %d\n", shared_data->curr);
        sem_post(&shared_data->write);
      }

      printf("Child exit\n");

      exit(0);
  }
  for (int i = 1; i <= 100; ++i) {
    sem_wait(&shared_data->write);
    printf("Parent shared write: %d\n", i);
    shared_data->curr = i;
    sem_post(&shared_data->read);
  }

  printf("Parent exit\n");

  sem_destroy(&shared_data->read);
  sem_destroy(&shared_data->write);
  munmap(shared_data, sizeof(struct shared_data));
  shm_unlink("/myshm");
}

// int x = 0;
// int* x_ptr = &x;

// int main() {
//   pid_t pid = fork();

//   int fd = shm_open("/myshm", O_RDWR | O_CREAT, 0666);
//   if (fd == -1) {
//     perror("shm_open");
//     exit(1);
//   }
//   if (ftruncate(fd, sizeof(int)) == -1) {
//     perror("ftruncate");
//     exit(1);
//   }

//   int* shared_x =
//       (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd,
//       0);

//   if (shared_x == MAP_FAILED) {
//     perror("mmap");
//     exit(1);
//   }

//   switch (pid) {
//     case -1:
//       perror("fork");
//       exit(1);
//     case 0:
//       // child
//       usleep(200);
//       printf("Child before: %d\n", *x_ptr);
//       *x_ptr = *shared_x;
//       printf("Child after: %d\n", *x_ptr);
//       exit(0);
//     default:
//       *x_ptr = 123;
//       wait(NULL);
//   }
// }