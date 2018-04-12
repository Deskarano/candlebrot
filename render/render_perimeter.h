//
// Created by grg on 4/25/17.
//

#ifndef CANDLEBROT_PERIMETER_RENDER_H
#define CANDLEBROT_PERIMETER_RENDER_H

#ifndef CANDLEBROT_RENDER_H
#include "render.h"
#endif

#ifndef CANDLEBROT_STACK_H
#include "../stack.h"
#endif

static int calculate_perimeter_pixel(fractal_t *fractal,
                                     render_t *render,
                                     int x, int y)
{
    if(!get_pixel_rendered(fractal, x, y))
    {
        render_pixel(fractal, render, x, y);
    }

    if(get_pixel_value(fractal, x, y))
    {
        return 0;
    }

    for(int dy = -1; dy <= 1; dy++)
    {
        for(int dx = -1; dx <= 1; dx++)
        {
            if(dy == 0 && dx == 0)
            {
                dx++;
            }

            if(x + dx >= 0 && x + dx < fractal->width &&
               y + dy >= 0 && y + dy < fractal->height)
            {
                if(!get_pixel_rendered(fractal, x + dx, y + dy))
                {
                    render_pixel(fractal, render, x + dx, y + dy);
                }

                if(get_pixel_value(fractal, x + dx, y + dy))
                {
                    set_pixel_perimeter(fractal, x, y, 1);
                    return 1;
                }
            }
            else
            {
                set_pixel_perimeter(fractal, x, y, 1);
                return 1;
            }
        }
    }

    return 0;
}

typedef struct
{
    int min_x;
    int max_x;

    int min_y;
    int max_y;

    int calls;
} perimeter_data_t;

static void render_perimeter_from_data(fractal_t *fractal, perimeter_data_t *data)
{
    for(int y = data->min_y; y <= data->max_y; y++)
    {
        int last_border = data->min_x;

        for(int x = data->min_x; x < data->max_x; x++)
        {
            if(get_pixel_rendered(fractal, x, y) &&
               get_pixel_rendered(fractal, x + 1, y))
            {
                if(get_pixel_value(fractal, x, y) &&
                   !get_pixel_value(fractal, x + 1, y))
                {
                    //don't render these points, there might be more perimeters in them
                    last_border = x;
                }

                if(!get_pixel_value(fractal, x, y) &&
                   get_pixel_value(fractal, x + 1, y))
                {
                    for(int sub_x = last_border; sub_x < x; sub_x++)
                    {
                        if(!get_pixel_rendered(fractal, sub_x, y))
                        {
                            set_pixel_value(fractal, sub_x, y, 0);
                            set_pixel_rendered(fractal, sub_x, y, 1);
                        }
                    }

                    last_border = x;
                }
            }
        }

        //take care of filling in everything past last_border
        if(get_pixel_value(fractal, last_border, y))
        {
            for(int dx = last_border; dx <= data->max_x; dx++)
            {
                set_pixel_value(fractal, dx, y, 0);
                set_pixel_rendered(fractal, dx, y, 1);
            }
        }

        //if the entire row needs to be filled in, do so
        //there will never be a case where a row is entirely outside since the perimeter tightly wraps the inside points
        if(last_border == data->min_x)
        {
            for(int dx = last_border; dx <= data->max_x; dx++)
            {
                set_pixel_value(fractal, dx, y, 0);
                set_pixel_rendered(fractal, dx, y, 1);
            }
        }
    }
}

typedef struct
{
    fractal_t *fractal;
    render_t *render;

    stack_element_t *point;
    stack_t *output_stack;

    __uint8_t working;
} render_perimeter_thread_arg;

static void *render_fractal_perimeter_thread(void *arg)
{
    fractal_t *fractal = ((render_perimeter_thread_arg *) arg)->fractal;
    render_t *render = ((render_perimeter_thread_arg *) arg)->render;
    stack_t *output_stack = ((render_perimeter_thread_arg *) arg)->output_stack;

    while(1)
    {
        while(((render_perimeter_thread_arg *) arg)->point == NULL);

        stack_element_t *point = ((render_perimeter_thread_arg *) arg)->point;

        if(point->x == -1 && point->y == -1)
        {
            return NULL;
        }

        for(int dy = -1; dy <= 1; dy++)
        {
            for(int dx = -1; dx <= 1; dx++)
            {
                if(dy == 0 && dx == 0)
                {
                    dx++;
                }

                if(point->x + dx >= 0 && point->x + dx < fractal->width &&
                   point->y + dy >= 0 && point->y + dy < fractal->height)
                {
                    calculate_perimeter_pixel(fractal, render, point->x + dx, point->y + dy);

                    if(get_pixel_perimeter(fractal, point->x + dx, point->y + dy)
                       && !get_pixel_traversed(fractal, point->x + dx, point->y + dy))
                    {
                        stack_element_t *new_point = malloc(sizeof(stack_element_t));
                        new_point->x = point->x + dx;
                        new_point->y = point->y + dy;

                        stack_push(output_stack, new_point);
                    }
                }
            }
        }

        free(point);
        ((render_perimeter_thread_arg *) arg)->point = NULL;
        ((render_perimeter_thread_arg *) arg)->working = 0;
    }
}

static void render_fractal_perimeter_controller(fractal_t *fractal,
                                                render_t *render,
                                                perimeter_data_t *data,
                                                stack_t *point_stack)
{
    pthread_t *threads = malloc(render->num_threads * sizeof(pthread_t));
    render_perimeter_thread_arg *args = malloc(render->num_threads * sizeof(render_perimeter_thread_arg));

    for(int i = 0; i < render->num_threads; i++)
    {
        args[i].fractal = fractal;
        args[i].render = render;
        args[i].point = NULL;
        args[i].output_stack = new_stack();
        args[i].working = 0;

        pthread_create(&threads[i], NULL, render_fractal_perimeter_thread, &args[i]);
    }

    while(point_stack->size > 0)
    {
        //assign points
        for(int i = 0; i < render->num_threads; i++)
        {
            if(!args[i].working && point_stack->size > 0)
            {
                stack_element_t *point = stack_pop(point_stack);

                //ignore duplicates
                while(get_pixel_traversed(fractal, point->x, point->y) && point_stack->size > 0)
                {
                    //printf("\tDumping point %p (%i, %i)\n", point, point->y, point->x);
                    free(point);
                    point = stack_pop(point_stack);
                }

                if(!get_pixel_traversed(fractal, point->x, point->y))
                {
                    set_pixel_traversed(fractal, point->x, point->y, 1);
                    data->calls++;

                    if(point->x < data->min_x)
                    {
                        data->min_x = point->x;
                    }

                    if(point->x > data->max_x)
                    {
                        data->max_x = point->x;
                    }

                    if(point->y < data->min_y)
                    {
                        data->min_y = point->y;
                    }

                    if(point->y > data->max_y)
                    {
                        data->max_y = point->y;
                    }

                    //printf("\tPushing point %p (%i, %i) to thread %i\n", point, point->y, point->x, i);
                    //printf("\t\t%zu points in stack\n", point_stack->size);
                    //printf("\t\t%i points assigned\n", data->calls);

                    args[i].working = 1;
                    args[i].point = point;
                }
            }
        }

        if(point_stack->size == 0)
        {
            for(int i = 0; i < render->num_threads; i++)
            {
                while(args[i].working);
            }
        }

        //drain finished points
        for(int i = 0; i < render->num_threads; i++)
        {
            if(!args[i].working && args[i].output_stack->size > 0)
            {
                while(args[i].output_stack->size > 0)
                {
                    stack_element_t *point = stack_pop(args[i].output_stack);
                    //printf("\tRetrieved point %p (%i, %i) from thread %i\n", point, point->y, point->x, i);
                    stack_push(point_stack, point);
                }
            }
        }
    }

    stack_element_t *kill_signal = malloc(sizeof(stack_element_t));
    kill_signal->x = -1;
    kill_signal->y = -1;

    for(int i = 0; i < render->num_threads; i++)
    {
        args[i].point = kill_signal;
        pthread_join(threads[i], NULL);

        free_stack(args[i].output_stack);
    }

    free(kill_signal);
    free(threads);
    free(args);
}

static void render_fractal_perimeter(fractal_t *fractal, render_t *render)
{
    int num_perimeters = 0;

    for(int y = 0; y < fractal->height; y++)
    {
        for(int x = 0; x < fractal->width; x++)
        {
            if(!get_pixel_rendered(fractal, x, y))
            {
                render_pixel(fractal, render, x, y);

                if(calculate_perimeter_pixel(fractal, render, x, y)
                   && !get_pixel_traversed(fractal, x, y))
                {
                    printf("Found candidate point (%i, %i)!\n", y, x);

                    perimeter_data_t *data = malloc(sizeof(perimeter_data_t));
                    data->min_x = x;
                    data->max_x = x;
                    data->min_y = y;
                    data->max_y = y;
                    data->calls = 0;

                    stack_element_t *point = malloc(sizeof(stack_element_t));
                    point->x = x;
                    point->y = y;

                    stack_t *point_stack = new_stack();
                    stack_push(point_stack, point);

                    render_fractal_perimeter_controller(fractal, render, data, point_stack);

                    if(data->min_x == data->max_x && data->min_y == data->max_y)
                    {

                    }
                    else
                    {
                        num_perimeters++;
                        printf("Found perimeter (%i, %i) -> (%i, %i) in %i calls (area %i)\n",
                               data->min_y, data->min_x, data->max_y, data->max_x, data->calls,
                               (data->max_x - data->min_x + 1) * (data->max_y - data->min_y + 1));

                        render_perimeter_from_data(fractal, data);
                    }

                    free(data);
                    free_stack(point_stack);

                    printf("Done analyzing point (%i, %i)!\n", y, x);
                }
            }
        }
    }

    printf("Found a total of %i perimeters\n", num_perimeters);
}

#endif //CANDLEBROT_PERIMETER_RENDER_H
