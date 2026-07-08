#ifndef WORKER_H
#define WORKER_H

#include <signal.h>
#include <math.h>
#include "img.h"

/*
 * Limite une valeur entière dans l’intervalle [0, 255].
 *
 * Cette fonction est utilisée pour s’assurer qu’une valeur de pixel
 * reste dans la plage valide des composantes d’une image.
 */
unsigned char clamp(int value);

/*
 * Cette fonction est exécutée par un thread et applique un filtre
 * d’image sur un segment précis de lignes de l’image partagée.
 * Chaque thread traite une portion distincte de l’image afin de
 * paralléliser le calcul.
 *
 * Les filtres supportés incluent la conversion en niveaux de gris,
 * le négatif et l’ajustement de la luminosité.
 */
void *worker_routine_thread(void *arg);

#endif
