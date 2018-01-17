#include "render/render_brute_force.h"
#include "render/render_perimeter.h"

#include "image.h"

static char *int_to_binary(__uint32_t n)
{
    char *name = malloc(33 * sizeof(char));

    for(int i = 0; i < 32; i++)
    {
        if(n & 1)
        {
            name[32 - i - 1] = '1';
        }
        else
        {
            name[32 - i - 1] = '0';
        }

        n = n >> 1;
    }

    name[32] = '\0';

    return name;
}

static unsigned int mandelbrot_2(double init_x,
                                 double init_y,
                                 int max_iterations)
{
    unsigned int i = 1;
    double x = 0;
    double y = 0;

    while(i < max_iterations)
    {
        double new_x = x * x - y * y + init_x;
        double new_y = 2 * x * y + init_y;

        x = new_x;
        y = new_y;

        if(x > 2 || y > 2)
        {
            return i;
        }

        i++;
    }

    return 0;
}

static unsigned int mandelbrot_3(double init_x,
                                 double init_y,
                                 int max_iterations)
{
    unsigned int i = 1;
    double x = 0;
    double y = 0;

    while(i < max_iterations)
    {
        double new_x = x * x * x - 3 * x * y * y + init_x;
        double new_y = 3 * x * x * y - y * y * y + init_y;

        x = new_x;
        y = new_y;

        if(x > 2 || y > 2)
        {
            return i;
        }

        i++;
    }

    return 0;
}

static void fractal_to_bitmap(fractal_t *fractal,
                              bitmap_t *bitmap)
{
    for(int y = 0; y < fractal->height; y++)
    {
        for(int x = 0; x < fractal->width; x++)
        {
            if(get_pixel_perimeter(fractal, x, y))
            {
                set_pixel(bitmap, x, y, 0xFF0000);
            }
            else
            {
                set_pixel(bitmap, x, y, get_pixel_value(fractal, x, y) * 512 + 0x00FF00);
            }
        }
    }
}

int main()
{
    fractal_t *fractal = new_fractal(5000, 5000, -2.5, -1.5, .5, 1.5);
    bitmap_t *bitmap = new_bitmap(5000, 5000);
    render_t *render = new_render(mandelbrot_2, 65536, 1);

    render_fractal_perimeter(fractal, render);
    fractal_to_bitmap(fractal, bitmap);
    save_png_to_file(bitmap, "/home/grg/outline.png");

    free_fractal(fractal);
    free_bitmap(bitmap);
    free_render(render);
}