#include <constants.h>
#include <rays.h>
#include <pthread.h>
#include "SDL.h"

// Structure of arguments needed for the parallel computing of 
// rays.
typedef struct args {
    int8_t *pixels;
    sphere *spheres;
    int n_spheres;

    // the tid is used for the calculatio of the portion of the
    // screen that will be calculated by that thread.
    uint8_t tid; 

} t_args;

// The job done by every thread
void* thread_job(void *a);

void parallel_render(int8_t *pixels, sphere *spheres, int n_spheres,
        pthread_t *threads);
