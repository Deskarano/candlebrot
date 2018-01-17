cmake_minimum_required(VERSION 3.6)
project(candlebrot)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-lpng -lpthread")

set(SOURCE_FILES
        main.c
        image.h
        fractal.h
        stack.h
        render/render.h
        render/render_brute_force.h
        render/render_perimeter.h)

add_executable(candlebrot ${SOURCE_FILES})