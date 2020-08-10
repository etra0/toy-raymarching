#include <stdio.h>
#include <stdlib.h>

#include <constants.h>
#include <rays.h>


#ifdef PARALLEL
#include <pthread.h>
#include <parallel_render.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL.h>

uint8_t init();

SDL_Window *g_window = NULL;
SDL_Renderer *renderer = NULL;


uint8_t init() {
    uint8_t success = 1;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("No se pudo inicializar SDL: %s\n", SDL_GetError());
        return 0;
    }

    g_window = SDL_CreateWindow("Ray casting", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!g_window) {
        printf("No se pudo crear la ventana: %s", SDL_GetError());
        return 0;
    }

    return 1;
}



void sequential_render(int8_t *pixels, sphere *spheres, int n_spheres, ray *rays, color *colors) {
    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
        const int x = i % (SCREEN_WIDTH);
        const int y = i / (SCREEN_WIDTH);
        const float current_value =
            (float)i / (float)(SCREEN_WIDTH * SCREEN_HEIGHT);

        const int c_i = (x + y*SCREEN_WIDTH)*4;

        ray *r = &rays[x + y*SCREEN_WIDTH];
        r->x = x;
        r->y = y;
        r->z = 0.;
        r->k = 5001.;
        r->alive = 1;

        color *c = &colors[x + y*SCREEN_WIDTH];
        ray_marching(r, spheres, n_spheres, c);
        pixels[c_i + 0] = c->R;
        pixels[c_i + 1] = c->G;
        pixels[c_i + 2] = c->B;
        pixels[c_i + 3] = SDL_ALPHA_OPAQUE;

    }
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Debe ingresar la cantidad de esferas\n");
        return EXIT_FAILURE;
    }

    SDL_SetMainReady();

    int n_spheres = atoi(argv[1]);

    if (!init()) {
        printf("No se pudo inicializar\n");
        return EXIT_FAILURE;
    }

    #ifdef PARALLEL
    printf("nthreads: %d\n", N_THREADS);
    #endif

    SDL_Renderer *renderer = SDL_CreateRenderer(g_window, -1,
        SDL_RENDERER_SOFTWARE);

    // Texture used for the 3d world simulation
    SDL_Texture *texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH, SCREEN_HEIGHT);

    int8_t *pixels = (int8_t *)malloc(
        SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(int8_t));
    
    uint8_t running = 1;
    SDL_Event event;

    sphere *spheres = initialize_spheres(n_spheres);
    color *colors = (color *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(color));;
    ray *rays = (ray *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(ray));;

    #ifdef PARALLEL
    pthread_t threads[N_THREADS];
    #endif

    uint32_t iters = 0;
    while(running) {
        uint64_t start = SDL_GetPerformanceCounter();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                break;
            }
        }

        move_spheres(spheres, n_spheres);

        #ifdef PARALLEL
        parallel_render(pixels, spheres, n_spheres, threads, rays, colors);
        #else
        sequential_render(pixels, spheres, n_spheres, rays, colors);
        #endif

        SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * 4);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // calculation of frames per second.
        uint64_t end = SDL_GetPerformanceCounter();
        double freq = (double)SDL_GetPerformanceFrequency();
        float secs = (float)(end - start) /(freq);
        printf("%f\n", 1/(secs));
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(g_window);

    free(spheres);

    SDL_Quit();
    return EXIT_SUCCESS;
}
