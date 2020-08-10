#include <stdio.h>
#include <stdlib.h>
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

struct arr{
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    sphere *spheres;
    int n_spheres;
    int8_t *pixels;
};

void foo(void* userData){
    struct arr *b = (struct arr*)userData;
    move_spheres(b->spheres, b->n_spheres);
    SDL_RenderClear(b->renderer);
    SDL_UpdateTexture(b->texture, NULL, b->pixels, SCREEN_WIDTH * 4);
    SDL_RenderCopy(b->renderer, b->texture, NULL, NULL);
    SDL_RenderPresent(b->renderer);
    return;
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

    int8_t *pixels;
    cudaMallocHost(&pixels,
        SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(int8_t));
    
    int8_t *pixels_dev;
    
    cudaMalloc((void **)&pixels_dev, SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(int8_t));

    uint8_t running = 1;
    SDL_Event event;

    sphere *spheres = initialize_spheres(n_spheres);
    sphere *spheres_async;
    cudaMallocHost((void**)&spheres_async,n_spheres * sizeof(sphere));

    memcpy(spheres_async, spheres, n_spheres * sizeof(sphere));

    sphere *spheres_dev;
    cudaMalloc((void **)&spheres_dev, n_spheres*sizeof(sphere));
    cudaMemcpy(spheres_dev,spheres,n_spheres*sizeof(sphere), cudaMemcpyHostToDevice);

    // alloc rays & colors
    color *colors;
    ray *rays;
    cudaMalloc((void **)&colors, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(color));
    cudaMalloc((void **)&rays, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(ray));

    uint32_t iters = 0;
    int block_size = 256, grid_size = (int)ceil((float)(SCREEN_HEIGHT*SCREEN_WIDTH)/256.);
    float secs = 0;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    cudaStream_t x,y;
    cudaStreamCreate(&x);
    cudaStreamCreate(&y);
    int off = 0;
    struct arr *arr;
    arr = new (struct arr);
    arr->pixels = pixels;
    arr->texture = texture;
    arr->renderer = renderer;
    arr->spheres = spheres_async;
    arr->n_spheres = n_spheres;
    // int size = SCREEN_HEIGHT*SCREEN_WIDTH;
    while(running) {
        // cudaStream_t us;
        // if (off%2) us = x;
        // else us = y;
        uint64_t start = SDL_GetPerformanceCounter();
        // //
        // //cudaMemcpy(dest, orig,  sizeof(), cudaMemcpyDeviceToHost);
        // //SDL_RenderClear(renderer);

        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                break;
            }
        }
        

        /**

        saaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        **/
        sequential_render<<<grid_size,block_size,0,off%2?x:y>>>(pixels_dev, spheres_dev, n_spheres, rays, colors);
        cudaMemcpyAsync(spheres_dev,spheres_async,n_spheres*sizeof(sphere), cudaMemcpyHostToDevice, off%2?x:y);
        cudaMemcpyAsync(pixels, pixels_dev, SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(int8_t), cudaMemcpyDeviceToHost,off%2?x:y);
        cudaLaunchHostFunc(off%2?x:y, (cudaHostFn_t)foo, (void*)arr);

        

        //cudaMemcpy(pixels, pixels_dev, SCREEN_WIDTH * SCREEN_HEIGHT * 4 * sizeof(int8_t), cudaMemcpyDeviceToHost);
        //sequential_render<<<grid_size,block_size>>>(pixels_dev, spheres_dev, n_spheres, rays, colors);

        

        // calculation of frames per second.
        uint64_t end = SDL_GetPerformanceCounter();
        double freq = (double)SDL_GetPerformanceFrequency();
        secs = (float)(end - start) /(freq);
        printf("%f\n", 1/(secs));
        off++;
        
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
