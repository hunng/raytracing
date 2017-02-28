#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define PTR 1
//#define OMP 1

#ifdef OMP
#include <omp.h>
#endif

#ifdef PTR
#include <pthread.h>
#endif

#include "primitives.h"
#include "raytracing.h"

#define OUT_FILENAME "out.ppm"

#define ROWS 512
#define COLS 512

static void write_to_ppm(FILE *outfile, uint8_t *pixels,
                         int width, int height)
{
    fprintf(outfile, "P6\n%d %d\n%d\n", width, height, 255);
    fwrite(pixels, 1, height * width * 3, outfile);
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

int main()
{

#ifdef OMP    /* number of child thread for openmp */
    int thread_count = 512;
#endif

    uint8_t *pixels;
    light_node lights = NULL;
    rectangular_node rectangulars = NULL;
    sphere_node spheres = NULL;
    color background = { 0.0, 0.1, 0.1 };
    struct timespec start, end;

#include "use-models.h"

    /* allocate by the given resolution */
    pixels = malloc(sizeof(unsigned char) * ROWS * COLS * 3);
    if (!pixels) exit(-1);

#ifdef PTR
    /* prepared pthread needed information */
    /* https://github.com/ierosodin/raytracing/blob/pthread/main.c */
    int pthread_count = 2;
    pthread_t *thread_handles;
    thread_handles = malloc(pthread_count * sizeof(pthread_t));

    inputneed *need = malloc(sizeof(inputneed));
    need->pthread_count = pthread_count;
    need->pixels = pixels;
    need->background_color[0] = background[0];
    need->background_color[1] = background[1];
    need->background_color[2] = background[2];
    need->rectangulars = rectangulars;
    need->spheres = spheres;
    need->lights = lights;
    need->view = &view;
    need->width = 512;
    need->height = 512;
#endif


    printf("# Rendering scene\n");
    /* do the ray tracing with the given geometry */
    clock_gettime(CLOCK_REALTIME, &start);
#ifdef OMP
    #pragma omp parallel num_threads(thread_count)
    raytracing(pixels, background, rectangulars, spheres, lights, &view, ROWS, COLS, 0, 512);
#elif defined(PTR)
    need->rank = 0;
    pthread_create(&thread_handles[0], NULL, raytracingWS, (void *) need);
    need->rank = 1;
    pthread_create(&thread_handles[1], NULL, raytracingWS, (void *) need);

    for (int i = 0; i < pthread_count; i++) {
        pthread_join(thread_handles[i], NULL);
    }
#else
    raytracing(pixels, background, rectangulars, spheres, lights, &view, ROWS, COLS);
#endif


    clock_gettime(CLOCK_REALTIME, &end);
    {
        FILE *outfile = fopen(OUT_FILENAME, "wb");
        write_to_ppm(outfile, pixels, ROWS, COLS);
        fclose(outfile);
    }

    delete_rectangular_list(&rectangulars);
    delete_sphere_list(&spheres);
    delete_light_list(&lights);
#ifdef PTR
    free(thread_handles);
    free(need);
#endif
    free(pixels);
    printf("Done!\n");
    printf("Execution time of raytracing() : %lf sec\n", diff_in_second(start, end));
    return 0;
}
