#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#define CHECK(x) if ((x) == -1) { perror(#x); exit(EXIT_FAILURE); }

#define TCHECK(op) \
  if ((op) != 0) { \
    fprintf(stderr, "Erreur Thread: %s\n", strerror(op)); \
    exit(EXIT_FAILURE); \
  }

#define MCHECK(ptr) \
  if ((ptr) == MAP_FAILED) { \
    perror("mmap"); \
    exit(EXIT_FAILURE); \
  }

#define MAX_IMAGE_SIZE (100 * 1024 * 1024)
#define SHM_REQUEST_NAME "/shm_requetes_img"
#define SEM_MUTEX_NAME   "/sem_mutex_img"
#define SEM_EMPTY_NAME   "/sem_empty_img"
#define SEM_FULL_NAME    "/sem_full_img"

#define MAX_PENDING_REQUESTS 10

#define FIFO_NAME_FMT "/tmp/fifo_rep_%d"

#define FILTER_GRAYSCALE 1
// Bonus possibles
#define FILTER_NEGATIVE  2
#define FILTER_BRIGHTNESS 3

struct filter_request {
  pid_t pid;
  char chemin[256];
  int filtre;
  int parametres[5];
};

struct shm_request_buffer {
  struct filter_request requests[MAX_PENDING_REQUESTS];
  int in;
  int out;
};

struct segment_image {
  int width;
  int height;
  int bpp;
  size_t size;

  unsigned char pixels[];
};

struct thread_workspace {
  int thread_id;
  struct segment_image *shm;
  int ligne_debut;
  int ligne_fin;
  int filtre;
  int parametre;
};

static inline void P(sem_t *sem) {
  CHECK(sem_wait(sem));
}

static inline void V(sem_t *sem) {
  CHECK(sem_post(sem));
}

#endif
