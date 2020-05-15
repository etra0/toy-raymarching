#include <constants.h>
#include <rays.h>
#include <pthread.h>
#include "SDL.h"

typedef struct args {
    int8_t *pixels;
    sphere **spheres;
    int n_spheres;
    uint8_t tid;
} t_args;

void* thread_job(void *a);

void parallel_render(int8_t *pixels, sphere *spheres, int n_spheres,
        pthread_t *threads, t_args args[N_THREADS]);
