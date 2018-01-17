#ifndef CANDLEBROT_RENDER_H
#define CANDLEBROT_RENDER_H

#ifndef _PTHREAD_H
#include <pthread.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef CANDLEBROT_FRACTAL_H
#include "../fractal.h"
#endif

typedef struct
{
    unsigned int (*render_function)(double, double, int);

    int max_iterations;
    int num_threads;
} render_t;

static render_t *new_render(unsigned int (*render_function)(double, double, int),
                            int max_iterations, int num_threads)
{
    render_t *render = malloc(sizeof(render_t));

    render->render_function = render_function;
    render->max_iterations = max_iterations;
    render->num_threads = num_threads;

    return render;
}

static void free_render(render_t *render)
{
    free(render);
}

static void render_pixel(fractal_t *fractal, render_t *render, int x, int y)
{
    __uint32_t value = render->render_function(fractal->x_1 + fractal->x_step * x,
                                               fractal->y_1 + fractal->y_step * y,
                                               render->max_iterations);

    set_pixel_value(fractal, x, y, value);
    set_pixel_rendered(fractal, x, y, 1);
}

#endif //CANDLEBROT_RENDER_H