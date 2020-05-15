#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <rays.h>
#include <SDL.h>

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;

uint8_t init();

void close();

SDL_Window *g_window = NULL;
SDL_Renderer *renderer = NULL;


uint8_t init() {
    uint8_t success = 1;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("No se pudo inicializar SDL: %s\n", SDL_GetError());
        return 0;
    }

    g_window = SDL_CreateWindow("Ray marching", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!g_window) {
        printf("No se pudo crear la ventana: %s", SDL_GetError());
        return 0;
    }

    return 1;
}

sphere** initialize_spheres(int n_spheres) {
    sphere **s;
    s = (sphere **)malloc(sizeof(sphere*)*n_spheres);

    // initialize random seed
    time_t t;
    srand((unsigned) time(&t));

    for (int i = 0; i < n_spheres; i++) {
        s[i] = (sphere *)malloc(sizeof(sphere));
        s[i]->x = rand() % SCREEN_WIDTH;
        s[i]->y = rand() % SCREEN_HEIGHT;
        s[i]->z = rand() % SCREEN_WIDTH + 50;
        // s[0]->z = 10.;

        s[i]->R = rand() % 255;
        s[i]->G = rand() % 255;
        s[i]->B = rand() % 255;

        s[i]->radius = s[i]->z - s[i]->z*0.1;
    }

    return s;
}

void sequential_render(int8_t *pixels, sphere **spheres, int n_spheres) {
    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
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

        color *c = ray_marching(r, spheres, n_spheres);
        pixels[c_i + 0] = c->R;
        pixels[c_i + 1] = c->G;
        pixels[c_i + 2] = c->B;
        pixels[c_i + 3] = SDL_ALPHA_OPAQUE;

        free(c);
        free(r);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Debe ingresar la cantidad de esferas\n");
        return EXIT_FAILURE;
    }

    int n_spheres = atoi(argv[1]);

    if (!init()) {
        printf("No se pudo inicializar\n");
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(g_window, -1,
        SDL_RENDERER_ACCELERATED);

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    int8_t *pixels = (int8_t *)malloc(
        SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(int8_t));
    
    uint8_t running = 1;
    SDL_Event event;

    sphere **spheres = initialize_spheres(n_spheres);

    while(running) {
        const uint64_t start = SDL_GetPerformanceCounter();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // polling event 
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_UP]) spheres[0]->z += 2.;
        if (keys[SDL_SCANCODE_DOWN]) spheres[0]->z -= 2.;
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                break;
            }
        }

        sequential_render(pixels, spheres, n_spheres);

        SDL_UpdateTexture(texture, NULL, &pixels[0], SCREEN_WIDTH * 4);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        const uint8_t end = SDL_GetPerformanceCounter();
        const uint8_t freq = SDL_GetPerformanceFrequency();
        const double secs = (end - start) /((double)freq);
        printf("%lf\n", secs*100);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(g_window);

    for (int i = 0; i < n_spheres; free(spheres[i]), i++);
    free(spheres);

    SDL_Quit();
    return EXIT_SUCCESS;
}