#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define BUFSIZE 7
#define LOOPS 50

typedef struct shared_mem {
  char buffer[BUFSIZE];
} shared_mem_t;

void writer(shared_mem_t* mem) {
  sem_t* read_s = sem_open("/read_s", O_CREAT | O_RDONLY, 0644, 0);
  if (read_s == SEM_FAILED) {
    perror("sem_open");
    return;
  }
  sem_t* write_s = sem_open("/write_s", O_CREAT | O_WRONLY, 0644, BUFSIZE);
  if (write_s == SEM_FAILED) {
    perror("sem_open");
    return;
  }

  for (size_t i = 0; i < LOOPS; ++i) {
    sem_wait(write_s);
    mem->buffer[i % BUFSIZE] = i;
    sem_post(read_s);
  }

  sem_close(read_s);
  sem_close(write_s);
  sem_unlink("/read_s");
  sem_unlink("/write_s");
}

void reader(shared_mem_t* mem) {
  sem_t* read_s = sem_open("/read_s", O_CREAT | O_RDONLY, 0644, 0);
  if (read_s == SEM_FAILED) {
    perror("sem_open");
    return;
  }
  sem_t* write_s = sem_open("/write_s", O_CREAT | O_WRONLY, 0644, BUFSIZE);
  if (write_s == SEM_FAILED) {
    perror("sem_open");
    return;
  }

  getc(stdin);

  for (size_t i = 0; i < LOOPS; ++i) {
    sem_wait(read_s);
    printf("%d\n", mem->buffer[i % BUFSIZE]);
    sem_post(write_s);
  }

  sem_close(read_s);
  sem_close(write_s);
  sem_unlink("/read_s");
  sem_unlink("/write_s");
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s <reader|writer>\n", argv[0]);
    return 1;
  }

  bool is_reader;
  if (strcmp(argv[1], "reader") == 0) {
    is_reader = true;
  } else if (strcmp(argv[1], "writer") == 0) {
    is_reader = false;
  } else {
    printf("Usage: %s <reader|writer>\n", argv[0]);
    return 1;
  }

  int fd = shm_open("/shared_mem", O_CREAT | O_RDWR, 0644);
  if (fd == -1) {
    perror("shm_open");
    return 1;
  }

  if (ftruncate(fd, sizeof(shared_mem_t)) == -1) {
    perror("ftruncate");
    return 1;
  }

  shared_mem_t* mem =
      mmap(NULL, sizeof(shared_mem_t), is_reader ? PROT_READ : PROT_WRITE,
           MAP_SHARED, fd, 0);
  if (mem == MAP_FAILED) {
    perror("mmap");
    return 1;
  }

  if (is_reader) {
    reader(mem);
  } else {
    writer(mem);
  }

  munmap(mem, sizeof(shared_mem_t));
  shm_unlink("/shared_mem");
  return 0;
}
