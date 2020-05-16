#include <stdlib.h>
#include <stdint.h>
#pragma once

// Ray traced from the camera to the objects with a threshold of 100 points
// of distance
typedef struct _ray {
    uint8_t alive;
    float x;
    float y;
    float z;
    float intensity;
    float k;
} ray;

// Sphere with coords c_x, c_y, c_z and radius to simulate a real world
// sphere, and RGB to calculate the intensity of the colour returned
// by the ray casting algorithm.
typedef struct _sphere {
    float x;
    float y;
    float z;
    float radius;

    int R;
    int G;
    int B;
} sphere;

// The ending structure to load to the viewer.
typedef struct _color {
    int R;
    int G;
    int B;
} color;

// Algorithm that throws the ray in a coord X, Y, where the Z is moving
// forward. at every new Z the algorithm checks if a sphere is impacted
// if it does, the algorithm stops and returns the new color
// if z get bigger than 100, the algorithm returns a black color which
// means it didn't impacted anything
color* ray_marching(ray *r, sphere *spheres, int n_spheres);

// Check if a sphere is impacted using the known formula to check
// if a point is inside a circle/sphere
uint8_t check_impact(sphere *s, float x, float y, float z);
