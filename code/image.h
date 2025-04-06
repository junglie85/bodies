#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef enum image_format_t image_format_t;
enum image_format_t
{
    IMAGE_FORMAT_R8G8B8A8_UNORM,
};

typedef struct image_t image_t;
struct image_t
{
    void *data;
    int32_t width;
    int32_t height;
    int32_t pitch;
    image_format_t format;
};

image_t load_image(const char *filename);

void free_image(image_t *image);

int32_t image_bytes_per_pixel(const image_t *image);

#endif // IMAGE_H
