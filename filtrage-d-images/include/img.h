#ifndef BMP_H
#define BMP_H

#include <stdint.h>
#include "common.h"

/*
 * Cette fonction utilise la bibliothèque stb_image pour lire différents formats
 * (JPG, PNG, BMP) et les convertir uniformément en une matrice de pixels RGB
 * (24 bits).
 * Elle alloue dynamiquement une structure `segment_image` contenant les
 * métadonnées
 * et les données brutes des pixels.
 */
struct segment_image *img_read(const char *path);

/*
 * Cette fonction exporte les données de pixels contenues dans la structure
 * `segment_image`
 * vers un fichier physique. Elle standardise la sortie au format PNG (Portable
 * Network Graphics)
 * pour garantir une compression sans perte de qualité après le traitement.
 */
int img_write(const char *filename, struct segment_image *img);
#endif
