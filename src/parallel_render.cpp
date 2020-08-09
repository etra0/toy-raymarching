#include <parallel_render.h>

// the SCREEN is divided by the amount of  threads, and so every thread
// is in charge to render a portion of the screen, speeding up the rendering
// process.
void* thread_job(void *a) {
    const int screen_size = SCREEN_WIDTH * SCREEN_HEIGHT;
    const int iters = (screen_size)/N_THREADS;

    // casting arguments
    t_args *args = (t_args *)a;

    int s_index = args->tid * iters;
    int e_index = s_index + iters;
    for (int i = s_index; i < e_index && i < screen_size; i++) {
        const int x = i % (SCREEN_WIDTH);
        const int y = i / (SCREEN_WIDTH);
        const float current_value = 
            (float)i / (float)(SCREEN_WIDTH * SCREEN_HEIGHT);

        const int c_i = (x + y*SCREEN_WIDTH)*4;

        ray *r = malloc(sizeof(ray));
        r->x = x;
        r->y = y;
        r->z = 0.;
        r->k = 5000.;
        r->alive = 1;

        sphere *current = (args->spheres);

        color *c = ray_marching(r, current, args->n_spheres);
        args->pixels[c_i + 0] = c->R;
        args->pixels[c_i + 1] = c->G;
        args->pixels[c_i + 2] = c->B;
        args->pixels[c_i + 3] = SDL_ALPHA_OPAQUE;

        free(c);
        free(r);
    }
    pthread_exit(NULL);
}

void parallel_render(int8_t *pixels, sphere *spheres, int n_spheres,
    pthread_t *threads) {

    t_args *args[N_THREADS];
    for (int i = 0; i < N_THREADS; i++) {

        // allocation of args per thread
        args[i] = malloc(sizeof(t_args));
        args[i]->pixels = pixels;
        args[i]->spheres = spheres;
        args[i]->n_spheres = n_spheres;
        args[i]->tid = i;

        pthread_create(&threads[i], NULL, thread_job, (void *)args[i]);
    }

    for (int i = 0; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
        free(args[i]);
    }
}
