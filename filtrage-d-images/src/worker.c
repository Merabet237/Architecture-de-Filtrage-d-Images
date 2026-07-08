/*
 * Programme worker de traitement d’images.
 *
 * Ce programme est lancé par le serveur pour traiter une requête
 * envoyée par un client. Il reçoit en paramètres :
 * - le chemin de l’image à traiter,
 * - le PID du client demandeur,
 * - le code du filtre à appliquer,
 * - un paramètre optionnel associé au filtre.
 *
 * Le worker charge l’image en mémoire partagée, puis crée plusieurs
 * threads afin de paralléliser l’application du filtre sur différentes
 * portions de l’image.
 *
 * Une fois le traitement terminé, l’image modifiée est envoyée au
 * client via une FIFO nommée basée sur le PID du client.
 * Le programme se termine après avoir libéré les ressources utilisées.
 */

#include "worker.h"

int main(int argc, char *argv[]) {
  if (argc < 5) {
    exit(EXIT_FAILURE);
  }
  char *chemin_image = argv[1];
  pid_t client_pid = atoi(argv[2]);
  int code_filtre = atoi(argv[3]);
  int valeur_param = atoi(argv[4]);
  printf("Worker %d: Job pour Client %d : %s (filtre %d, parametre %d)\n",
      getpid(), client_pid, chemin_image, code_filtre, valeur_param);
  struct segment_image *shared_img = img_read(chemin_image);
  int nb_threads = 8;
  pthread_t threads[nb_threads];
  struct thread_workspace workspace[nb_threads];
  int rows_per_thread = shared_img->height / nb_threads;
  for (int i = 0; i < nb_threads; i++) {
    workspace[i].thread_id = i;
    workspace[i].shm = shared_img;
    workspace[i].filtre = code_filtre;
    workspace[i].parametre = valeur_param;
    workspace[i].ligne_debut = i * rows_per_thread;
    workspace[i].ligne_fin
      = (i
        == nb_threads - 1) ? shared_img->height : (i + 1) * rows_per_thread;
    TCHECK(pthread_create(&threads[i], NULL, worker_routine_thread,
        &workspace[i]));
  }
  for (int i = 0; i < nb_threads; i++) {
    TCHECK(pthread_join(threads[i], NULL));
  }
  char fifo_name[256];
  snprintf(fifo_name, sizeof(fifo_name), FIFO_NAME_FMT, client_pid);
  signal(SIGPIPE, SIG_IGN);
  int res = img_write(fifo_name, shared_img);
  if (res == -1) {
    perror("write");
    exit(EXIT_FAILURE);
  } else if (res == 1) {
    perror("write");
    exit(EXIT_FAILURE);
  }
  free(shared_img);
  printf("Worker %d: Terminé.\n", getpid());
  return EXIT_SUCCESS;
}

unsigned char clamp(int value) {
  if (value < 0) {
    return 0;
  }
  if (value > 255) {
    return 255;
  }
  return (unsigned char) value;
}

void *worker_routine_thread(void *arg) {
  struct thread_workspace *ws = (struct thread_workspace *) arg;
  struct segment_image *si = ws->shm;
  unsigned char *p = si->pixels;
  int width = si->width;
  printf("Thread %d: Lignes %d à %d\n",
      ws->thread_id, ws->ligne_debut, ws->ligne_fin);
  for (int y = ws->ligne_debut; y < ws->ligne_fin; y++) {
    for (int x = 0; x < width; x++) {
      long index = (y * width + x) * 3;
      unsigned char r = p[index];
      unsigned char g = p[index + 1];
      unsigned char b = p[index + 2];
      if (ws->filtre == FILTER_GRAYSCALE) {
        unsigned char gray
          = (unsigned char) (0.299 * r + 0.587 * g + 0.114 * b);
        p[index] = gray;
        p[index + 1] = gray;
        p[index + 2] = gray;
      } else if (ws->filtre == FILTER_NEGATIVE) {
        p[index] = 255 - r;
        p[index + 1] = 255 - g;
        p[index + 2] = 255 - b;
      } else if (ws->filtre == FILTER_BRIGHTNESS) {
        p[index] = clamp(r + ws->parametre);
        p[index + 1] = clamp(g + ws->parametre);
        p[index + 2] = clamp(b + ws->parametre);
      }
    }
  }
  pthread_exit(NULL);
}
