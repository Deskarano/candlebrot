#ifndef CANDLEBROT_FRACTAL_H
#define CANDLEBROT_FRACTAL_H

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

typedef struct
{
    size_t width;
    size_t height;

    double x_1;
    double y_1;
    double x_2;
    double y_2;

    double x_step;
    double y_step;

    /* From LSB to MSB
     * Bit 1: 1 if rendered, 0 if not
     *     2: 1 if perimeter pixel, 0 if not
     *     3: 1 if pixel has been traversed when analyzing perimeter, 0 if not
     *     4 - 8: unused
     *     9 - 32: rendered value of pixel
     */
    __uint32_t **px_data;

    int last_row_rendered;
} fractal_t;

static fractal_t *new_fractal(size_t width, size_t height,
                              double x_1, double y_1,
                              double x_2, double y_2)
{
    fractal_t *fractal = malloc(sizeof(fractal_t));

    fractal->width = width;
    fractal->height = height;
    fractal->last_row_rendered = 0;

    if(x_1 < x_2)
    {
        fractal->x_1 = x_1;
        fractal->x_2 = x_2;
    }
    else
    {
        fractal->x_1 = x_2;
        fractal->x_2 = x_1;
    }

    if(y_1 < y_2)
    {
        fractal->y_1 = y_1;
        fractal->y_2 = y_2;
    }
    else
    {
        fractal->y_1 = y_2;
        fractal->y_2 = y_1;
    }

    fractal->x_step = (fractal->x_2 - fractal->x_1) / width;
    fractal->y_step = (fractal->y_2 - fractal->y_1) / height;

    fractal->px_data = calloc(fractal->height, sizeof(__uint32_t *));

    for(int y = 0; y < fractal->height; y++)
    {
        fractal->px_data[y] = calloc(fractal->width, sizeof(__uint32_t));
    }

    return fractal;
}

static void free_fractal(fractal_t *fractal)
{
    for(int y = 0; y < fractal->height; y++)
    {
        free(fractal->px_data[y]);
    }

    free(fractal->px_data);
    free(fractal);
}

static __uint32_t get_pixel_rendered(fractal_t *fractal, int x, int y)
{
    return fractal->px_data[y][x] & 1;
}

static __uint32_t get_pixel_perimeter(fractal_t *fractal, int x, int y)
{
    return (fractal->px_data[y][x] >> 1) & 1;
}

static __uint32_t get_pixel_traversed(fractal_t *fractal, int x, int y)
{
    return (fractal->px_data[y][x] >> 2) & 1;
}

static __uint32_t get_pixel_value(fractal_t *fractal, int x, int y)
{
    return fractal->px_data[y][x] >> 8;
}

static void set_pixel_rendered(fractal_t *fractal, int x, int y, __uint32_t value)
{
    fractal->px_data[y][x] &= 0xFFFFFFFF - 1;   //0b11111111111111111111111111111110
    fractal->px_data[y][x] += (value & 1);
}

static void set_pixel_perimeter(fractal_t *fractal, int x, int y, __uint32_t value)
{
    fractal->px_data[y][x] &= 0xFFFFFFFF - 2;   //0b11111111111111111111111111111101
    fractal->px_data[y][x] += ((value & 1) << 1);
}

static void set_pixel_traversed(fractal_t *fractal, int x, int y, __uint32_t value)
{
    fractal->px_data[y][x] &= 0xFFFFFFFF - 4;   //0b11111111111111111111111111111011
    fractal->px_data[y][x] += ((value & 1) << 2);
}

static void set_pixel_value(fractal_t *fractal, int x, int y, __uint32_t value)
{
    fractal->px_data[y][x] &= 0x000000FF;       //0b00000000000000000000000011111111
    fractal->px_data[y][x] += ((value & 0xFFFFFF) << 8);
}

#endif //CANDLEBROT_FRACTAL_H