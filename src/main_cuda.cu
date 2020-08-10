#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <rays.h>

#include <constants.h>

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
__device__ inline void check_impact_c(sphere *s, float x, float y, float z, int* F) {
    float xd = x - (s->x);
    float yd = y - (s->y);
    float zd = z - (s->z);

    float d = (xd*xd + yd*yd + zd*zd);
    float r = (s->radius) * (s->radius);

    *F = (int)(d <= r);

    return;
}


__device__ void ray_marching_c(ray *r, sphere *spheres, int n_spheres, color *c) {
    do {
        for (int i = 0; i < n_spheres; i++) {
            int F;
            check_impact_c(&spheres[i], r->x, r->y, r->z, &F);

            if (F) {
                float dz = (r->z);
                r->intensity = (r->k)/(dz*dz);
                c->R = min(spheres[i].R * r->intensity, 255.);
                c->G = min(spheres[i].G * r->intensity, 255.);
                c->B = min(spheres[i].B * r->intensity, 255.);
                return;
            }
        }

        r->z += 1;
        r->alive = r->z < 100;
    } while(r->alive);
}

sphere* initialize_spheres(int n_spheres) {
    sphere *s;
    s = (sphere *)malloc(sizeof(sphere)*n_spheres);

    // initialize random seed
    time_t t;
    srand((unsigned) time(&t));

    for (int i = 0; i < n_spheres; i++) {
        s[i].x = rand() % SCREEN_WIDTH;
        s[i].y = rand() % SCREEN_HEIGHT;
        s[i].z = rand() % SCREEN_WIDTH + 50;
        // s[0]->z = 10.;

        s[i].R = rand() % 255;
        s[i].G = rand() % 255;
        s[i].B = rand() % 255;

        s[i].radius = s[i].z - s[i].z*0.1;
    }

    return s;
}

__global__ void sequential_render(int8_t *pixels, sphere *spheres, int n_spheres, ray *rays, color *colors) {
    int tid= threadIdx.x + blockDim.x * blockIdx.x;
    const int x = tid % (SCREEN_WIDTH);
    const int y = tid / (SCREEN_WIDTH);

    const int c_i = tid*4;

    if (tid < SCREEN_WIDTH * SCREEN_HEIGHT) {
      ray *r = &rays[tid];
      r->x = x;
      r->y = y;
      r->z = 0.;
      r->k = 5001.;
      r->alive = 1;
      color *c = &colors[tid];
      c->R = 0; c->B = 0; c->G=0;
      ray_marching_c(r, spheres, n_spheres, c);
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

    SDL_Renderer *renderer = SDL_CreateRenderer(g_window, -1,
        SDL_RENDERER_SOFTWARE);

    // Texture used for the 3d world simulation
    SDL_Texture *texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH, SCREEN_HEIGHT);

    int8_t *pixels = (int8_t *)malloc(
        SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(int8_t));
    
    int8_t *pixels_dev;
    
    cudaMalloc((void **)&pixels_dev, SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(int8_t));

    uint8_t running = 1;
    SDL_Event event;

    sphere *spheres = initialize_spheres(n_spheres);

    sphere *spheres_dev;
    cudaMalloc((void **)&spheres_dev, n_spheres*sizeof(sphere));
    cudaMemcpy(spheres_dev,spheres,n_spheres*sizeof(sphere), cudaMemcpyHostToDevice);
    free(spheres);

    // alloc rays & colors
    color *colors;
    ray *rays;
    cudaMalloc((void **)&colors, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(color));
    cudaMalloc((void **)&rays, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(ray));

    uint32_t iters = 0;
    int block_size = 256, grid_size = (int)ceil((float)SCREEN_HEIGHT*SCREEN_WIDTH/256.);
    float secs = 0;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    while(running ) {
        uint64_t start = SDL_GetPerformanceCounter();

        //
        SDL_RenderClear(renderer);

        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                break;
            }
        }
        //cudaMemcpy(dest, orig,  sizeof(), cudaMemcpyDeviceToHost);
        sequential_render<<<grid_size,block_size>>>(pixels_dev, spheres_dev, n_spheres, rays, colors);
        cudaMemcpy(pixels, pixels_dev, SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(int8_t), cudaMemcpyDeviceToHost);

        SDL_UpdateTexture(texture, NULL, &pixels[0], SCREEN_WIDTH * 4);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // calculation of frames per second.
        uint64_t end = SDL_GetPerformanceCounter();
        double freq = (double)SDL_GetPerformanceFrequency();
        secs = (float)(end - start) /(freq);
        printf("%f\n", 1/(secs));
        
    }
    printf("%f\n", 1/(secs));

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(g_window);

    cudaFree(spheres_dev);
    cudaFree(pixels_dev);
    cudaFree(rays);
    cudaFree(colors);

    SDL_Quit();
    return EXIT_SUCCESS;
}
