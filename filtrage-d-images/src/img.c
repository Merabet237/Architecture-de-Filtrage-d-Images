#include "img.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct segment_image* img_read(const char *chemin) {
    int width;
    int height;
    int channels;

    unsigned char *data = stbi_load(chemin , &width, &height, &channels, 3);

    if (data == NULL) {
       fprintf(stderr, "Erreur chargement image : %s\n", stbi_failure_reason());
        exit(EXIT_FAILURE);
    }

    size_t data_taille = (size_t) (width * height * 3);
    if (data_taille > MAX_IMAGE_SIZE) {
        fprintf(stderr, "Image trop grande (>100M)\n");
        stbi_image_free(data);
        exit(EXIT_FAILURE);
    }

    struct segment_image *p = malloc(sizeof(struct segment_image) + data_taille);
    if (p == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    p -> width = width;
    p -> height = height;
    p -> bpp = 24;
    p -> size = data_taille;

    memcpy(p -> pixels, data, data_taille);

    stbi_image_free(data);

    return p;
}

int img_write(const char *filename, struct segment_image *img) {
    return stbi_write_png(filename, img->width, img->height, 3, img->pixels, img->width * 3);
}
