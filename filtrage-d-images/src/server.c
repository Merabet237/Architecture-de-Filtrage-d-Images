/*
 * Programme serveur de traitement d’images.
 *
 * Ce programme implémente le serveur d’une architecture client/serveur.
 * Il reçoit des requêtes de traitement d’images via une mémoire partagée
 * synchronisée par des sémaphores.
 *
 * Le serveur :
 * - initialise les gestionnaires de signaux (SIGINT pour l’arrêt propre,
 *   SIGCHLD pour la gestion des processus fils),
 * - crée et initialise la mémoire partagée servant de buffer de requêtes,
 * - met en place les sémaphores assurant l’exclusion mutuelle et la
 *   synchronisation producteur/consommateur,
 * - attend en boucle la réception de requêtes envoyées par les clients.
 *
 * Pour chaque requête reçue, le serveur crée un processus fils qui exécute
 * un programme travailleur (worker) chargé d’appliquer le filtre demandé
 * sur l’image. Le résultat est ensuite renvoyé au client via une FIFO
 * dédiée.
 *
 * Le serveur reste actif jusqu’à réception d’un signal d’interruption,
 * auquel cas il libère proprement l’ensemble des ressources IPC utilisées.
 */

#include "server.h"

int main(void) {
  struct sigaction sa_int, sa_chld;
  sa_int.sa_handler = handle_sigint;
  sigemptyset(&sa_int.sa_mask);
  sa_int.sa_flags = 0;
  CHECK(sigaction(SIGINT, &sa_int, NULL));
  sa_chld.sa_handler = handle_sigchld;
  sigemptyset(&sa_chld.sa_mask);
  sa_chld.sa_flags = SA_RESTART;
  CHECK(sigaction(SIGCHLD, &sa_chld, NULL));
  printf("serveur: demarrage....\n");
  shm_unlink(SHM_REQUEST_NAME);
  shm_fd_global = shm_open(SHM_REQUEST_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
  CHECK(shm_fd_global);
  CHECK(ftruncate(shm_fd_global, sizeof(struct shm_request_buffer)));
  buffer_global = mmap(NULL, sizeof(struct shm_request_buffer),
      PROT_READ | PROT_WRITE,
      MAP_SHARED, shm_fd_global, 0);
  MCHECK(buffer_global);
  buffer_global->in = 0;
  buffer_global->out = 0;
  sem_unlink(SEM_MUTEX_NAME);
  sem_unlink(SEM_EMPTY_NAME);
  sem_unlink(SEM_FULL_NAME);
  sem_mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);
  sem_empty = sem_open(SEM_EMPTY_NAME, O_CREAT, 0666, MAX_PENDING_REQUESTS);
  sem_full = sem_open(SEM_FULL_NAME, O_CREAT, 0666, 0);
  if (sem_mutex == SEM_FAILED || sem_empty == SEM_FAILED
      || sem_full == SEM_FAILED) {
    perror("sem_open creation");
    handle_sigint(0);
  }
  printf("serveur: pret a recevoir des requetes.\n");
  while (1) {
    P(sem_full);
    P(sem_mutex);
    int index = buffer_global->out;
    struct filter_request req = buffer_global->requests[index];
    buffer_global->out = (buffer_global->out + 1) % MAX_PENDING_REQUESTS;
    V(sem_mutex);
    V(sem_empty);
    printf("Serveur: Requête reçue : Client %d, Image %s\n", req.pid,
        req.chemin);
    switch (fork()) {
      case -1:
        perror("fork");
        exit(EXIT_FAILURE);
      case 0:
        char arg_pid[32];
        char arg_filtre[32];
        char arg_parametre[32];
        snprintf(arg_pid, sizeof(arg_pid), "%d", req.pid);
        snprintf(arg_filtre, sizeof(arg_filtre), "%d", req.filtre);
        snprintf(arg_parametre, sizeof(arg_parametre), "%d", req.parametres[0]);
        execl("./build/worker", "worker", req.chemin, arg_pid, arg_filtre,
            arg_parametre, NULL);
        perror("execl worker failed");
        exit(EXIT_FAILURE);
      default:
    }
  }
  return EXIT_SUCCESS;
}

void handle_sigint(int sig) {
  (void) sig;
  printf("arret demande, nettoyage commencer \n");
  if (sem_mutex != NULL) {
    sem_close(sem_mutex);
  }
  if (sem_empty != NULL) {
    sem_close(sem_empty);
  }
  if (sem_full != NULL) {
    sem_close(sem_full);
  }
  sem_unlink(SEM_MUTEX_NAME);
  sem_unlink(SEM_EMPTY_NAME);
  sem_unlink(SEM_FULL_NAME);
  if (buffer_global) {
    munmap(buffer_global, sizeof(struct shm_request_buffer));
  }
  if (shm_fd_global != -1) {
    close(shm_fd_global);
  }
  shm_unlink(SHM_REQUEST_NAME);
  printf("nettoyage terminer\n");
  exit(EXIT_SUCCESS);
}

void handle_sigchld(int sig) {
  (void) sig;
  while (waitpid(-1, NULL, WNOHANG) > 0) {
    printf("enfant terminé\n");
  }
}
