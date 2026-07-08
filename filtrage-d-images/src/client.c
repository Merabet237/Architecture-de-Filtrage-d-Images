/*
 * Programme client de traitement d’images.
 * Ce programme agit comme un client dans une architecture client/serveur.
 * Il envoie au serveur une requête de traitement d’image via une mémoire
 * partagée synchronisée par des sémaphores.
 * Le client fournit :
 * - le chemin d’une image à traiter,
 * - un code de filtre (valeur entre 1 et 3),
 * - un paramètre optionnel associé au filtre.
 * La requête est placée dans un buffer partagé. Le serveur applique le filtre
 * demandé et renvoie l’image traitée au client via une FIFO nommée, créée
 * spécifiquement pour ce client à partir de son PID.
 * L’image résultante est enregistrée localement dans un fichier de sortie.
 * Le programme assure également la synchronisation, la gestion des erreurs
 * et la libération des ressources IPC utilisées (FIFO, sémaphores, mémoire
 * partagée).
 */

#include "common.h"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Erreur: missed arguments");
    exit(EXIT_FAILURE);
  }
  const char *chemin_image = argv[1];
  int filtre = atoi(argv[2]);
  if (filtre != 1 && filtre != 2 && filtre != 3) {
    fprintf(stderr,
        "le code entré ne sagit pas d'un filtre veillez entre une valeur valide entre 1 a 3 \n");
    exit(EXIT_FAILURE);
  }
  int parametre = (argc >= 4) ? atoi(argv[3]) : 0;
  if (access(chemin_image, F_OK | R_OK) == -1) {
    fprintf(stderr, "Erreur: l'image n'existe pas");
    exit(EXIT_FAILURE);
  }
  pid_t mon_pid = getpid();
  char fifo_name[256];
  snprintf(fifo_name, sizeof(fifo_name), FIFO_NAME_FMT, mon_pid);
  if (mkfifo(fifo_name, 0666) == -1) {
    if (errno != EEXIST) {
      perror("mkfifo");
      exit(EXIT_FAILURE);
    }
  }
  int shm_fd = shm_open(SHM_REQUEST_NAME, O_RDWR, 0666);
  CHECK(shm_fd);
  struct shm_request_buffer *buffer
    = mmap(NULL, sizeof(struct shm_request_buffer),
      PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  MCHECK(buffer);
  sem_t *sem_mutex = sem_open(SEM_MUTEX_NAME, 0);
  sem_t *sem_empty = sem_open(SEM_EMPTY_NAME, 0);
  sem_t *sem_full = sem_open(SEM_FULL_NAME, 0);
  if (sem_mutex == SEM_FAILED || sem_empty == SEM_FAILED
      || sem_full == SEM_FAILED) {
    perror("sem_open");
    unlink(fifo_name);
    exit(EXIT_FAILURE);
  }
  printf("Client %d : attente place...\n", mon_pid);
  P(sem_empty);
  P(sem_mutex);
  int index = buffer->in;
  buffer->requests[index].pid = mon_pid;
  buffer->requests[index].filtre = filtre;
  memset(buffer->requests[index].parametres, 0,
      sizeof(buffer->requests[index].parametres));
  buffer->requests[index].parametres[0] = parametre;
  strncpy(buffer->requests[index].chemin, chemin_image, 255);
  buffer->in = (buffer->in + 1) % MAX_PENDING_REQUESTS;
  printf("Client%d: requete envoyee.\n", mon_pid);
  V(sem_mutex);
  V(sem_full);
  int fifo_fd = open(fifo_name, O_RDONLY);
  CHECK(fifo_fd);
  char out_filname[300];
  char *extension = strrchr(chemin_image, '.');
  snprintf(out_filname, sizeof(out_filname), "out_%d.%s", mon_pid,
      extension + 1);
  int out_fd = open(out_filname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  CHECK(out_fd);
  char buf[4096];
  ssize_t n;
  while ((n = read(fifo_fd, buf, sizeof(buf))) > 0) {
    if (write(out_fd, buf, (size_t) n) != n) {
      perror("write");
      break;
    }
  }
  printf("(client) image recu -> %s\n", out_filname);
  close(fifo_fd);
  close(out_fd);
  unlink(fifo_name);
  sem_close(sem_mutex);
  sem_close(sem_empty);
  sem_close(sem_full);
  munmap(buffer, sizeof(struct shm_request_buffer));
  close(shm_fd);
  return EXIT_SUCCESS;
}
