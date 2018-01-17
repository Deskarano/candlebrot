#ifndef CANDLEBROT_IMAGE_H
#define CANDLEBROT_IMAGE_H

#ifndef PNG_H
#include <png.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

typedef struct
{
    __uint8_t red;
    __uint8_t green;
    __uint8_t blue;
} pixel_t;

typedef struct
{
    size_t width;
    size_t height;
    pixel_t **pixels;
} bitmap_t;

static bitmap_t *new_bitmap(size_t width, size_t height)
{
    bitmap_t *bitmap = malloc(sizeof(bitmap_t));

    bitmap->width = width;
    bitmap->height = height;
    bitmap->pixels = calloc(height, sizeof(pixel_t *));

    for(int y = 0; y < height; y++)
    {
        bitmap->pixels[y] = calloc(width, sizeof(pixel_t));
    }

    return bitmap;
}

static void free_bitmap(bitmap_t *bitmap)
{
    for(int y = 0; y < bitmap->height; y++)
    {
        free(bitmap->pixels[y]);
    }

    free(bitmap->pixels);
    free(bitmap);
}

static void set_pixel(bitmap_t *bitmap, int x, int y, __uint32_t color)
{
    bitmap->pixels[y][x].red = (__uint8_t) (color >> 16 & 0xFF);
    bitmap->pixels[y][x].green = (__uint8_t) (color >> 8 & 0xFF);
    bitmap->pixels[y][x].blue = (__uint8_t) (color & 0xFF);
}

static void save_png_to_file(bitmap_t *bitmap, char *path)
{
    FILE *image = fopen(path, "wb");

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    png_set_IHDR(png_ptr, info_ptr,
                 (png_uint_32) bitmap->width,
                 (png_uint_32) bitmap->height,
                 8 /* depth */,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_init_io(png_ptr, image);
    png_set_rows(png_ptr, info_ptr, (unsigned char **) bitmap->pixels);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(image);
}

#endif //CANDLEBROT_IMAGE_H