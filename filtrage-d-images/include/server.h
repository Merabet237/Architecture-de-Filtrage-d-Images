#ifndef SERVER_H
#define SERVER_H

#include "common.h"

int shm_fd_global = -1;
struct shm_request_buffer *buffer_global = NULL;
sem_t *sem_mutex = NULL;
sem_t *sem_empty = NULL;
sem_t *sem_full = NULL;

/*
 * Cette fonction est appelée lors de la réception d’un signal
 * d’interruption (Ctrl+C). Elle permet un arrêt propre du serveur
 * en libérant toutes les ressources IPC utilisées.
 * Elle ferme et supprime les sémaphores, détache et supprime la
 * mémoire partagée, puis termine l’exécution du programme.
 */
void handle_sigint(int sig);

/*
 * Cette fonction est appelée lorsqu’un processus fils se termine.
 * Elle permet de récupérer les processus fils terminés afin d’éviter
 * la création de processus zombies.
 */
void handle_sigchld(int sig);

#endif
