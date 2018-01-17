//
// Created by grg on 4/25/17.
//

#ifndef CANDLEBROT_BRUTE_FORCE_RENDER_H
#define CANDLEBROT_BRUTE_FORCE_RENDER_H

#ifndef CANDLEBROT_RENDER_H
#include "render.h"
#endif

typedef struct
{
    fractal_t *fractal;
    render_t *render;
} render_fractal_brute_force_thread_arg;

static void *render_fractal_brute_force_thread(void *args)
{
    fractal_t *fractal = ((render_fractal_brute_force_thread_arg *) args)->fractal;
    render_t *render = ((render_fractal_brute_force_thread_arg *) args)->render;

    int y = fractal->last_row_rendered++;

    while(y < fractal->height)
    {
        printf("Assigned row %i / %i (y = %f)\n", y, (int) fractal->height, fractal->y_1 + fractal->y_step * y);
        for(int x = 0; x < fractal->width; x++)
        {
            render_pixel(fractal, render, x, y);
        }

        y = fractal->last_row_rendered++;
    }

    return NULL;
}

static void render_fractal_brute_force(fractal_t *fractal,
                                       render_t *render)
{
    pthread_t *threads = malloc(render->num_threads * sizeof(pthread_t));
    render_fractal_brute_force_thread_arg *arg = malloc(sizeof(render_fractal_brute_force_thread_arg));

    arg->fractal = fractal;
    arg->render = render;

    for(int i = 0; i < render->num_threads; i++)
    {
        pthread_create(&threads[i], NULL, render_fractal_brute_force_thread, arg);
    }

    for(int i = 0; i < render->num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(arg);
}


#endif //CANDLEBROT_BRUTE_FORCE_RENDER_H
