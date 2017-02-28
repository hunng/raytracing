#ifndef __RAYTRACING_H
#define __RAYTRACING_H

#include "objects.h"
#include "idx_stack.h"
#include <stdint.h>

#ifdef OMP
#include <omp.h>
#endif

typedef struct __INPUT_NEED {
    uint8_t *pixels;
    color background_color;
    rectangular_node rectangulars;
    sphere_node spheres;
    light_node lights;
    const viewpoint *view;
    int width;
    int height;
} inputneed;

void raytracing(uint8_t *pixels, color background_color,
                rectangular_node rectangulars, sphere_node spheres,
                light_node lights, const viewpoint *view,
                int width, int height);
void *raytracingWS(inputneed *input);

#endif
