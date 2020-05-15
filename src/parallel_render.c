#include <parallel_render.h>

void* thread_job(void *a) {
    const int iters = (SCREEN_WIDTH * SCREEN_HEIGHT)/N_THREADS;

    // casting arguments
    t_args *args = (t_args *)a;

    int s_index = args->tid * iters;
    int e_index = s_index + iters;

    for (int i = s_index; i < e_index && i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        const int x = i % (SCREEN_WIDTH);
        const int y = i / (SCREEN_WIDTH);
        const float current_value = (float)i / (float)(SCREEN_WIDTH * SCREEN_HEIGHT);

        const int c_i = (x + y*SCREEN_WIDTH)*4;

        ray *r = (ray *)malloc(sizeof(ray));
        r->x = x;
        r->y = y;
        r->z = 0.;
        r->k = 5000.;
        r->alive = 1;

        color *c = ray_marching(r, args->spheres, args->n_spheres);
        args->pixels[c_i + 0] = c->R;
        args->pixels[c_i + 1] = c->G;
        args->pixels[c_i + 2] = c->B;
        args->pixels[c_i + 3] = SDL_ALPHA_OPAQUE;

        free(c);
        free(r);
    }
    pthread_exit(NULL);
}

void parallel_render(int8_t *pixels, sphere **spheres, int n_spheres,
        pthread_t *threads, t_args **args) {
    for (int i = 0; i < N_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_job, (void *)args[i]);
    }

    for (int i = 0; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}
