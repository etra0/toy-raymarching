#include <stdint.h>
#include <stdlib.h>

typedef struct _ray {
    uint8_t alive;
    float x;
    float y;
    float z;
    float intensity;
    float k;
} ray;

typedef struct _sphere {
    float x;
    float y;
    float z;
    float radius;

    int R;
    int G;
    int B;
} sphere;

typedef struct _color {
    int R;
    int G;
    int B;
} color;

// check if an sphere was impacted
color* ray_marching(ray *r, sphere **spheres, int n_spheres);

uint8_t check_impact(sphere *s, float x, float y, float z);