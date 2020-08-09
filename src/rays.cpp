#include <rays.h>
#include <algorithm>

color* ray_marching(ray *r, sphere *spheres, int n_spheres) {
    color *c = (color *)malloc(sizeof(color));
    c->R = 0;
    c->G = 0;
    c->B = 0;

    do {
        for (int i = 0; i < n_spheres; i++) {
            const uint8_t impact = check_impact(&spheres[i], r->x, r->y, r->z);

            if (impact) {
                float dz = (r->z);
                r->intensity = (r->k)/(dz*dz);
                c->R = std::min<int>(spheres[i].R * r->intensity, 255);
                c->G = std::min<int>(spheres[i].G * r->intensity, 255);
                c->B = std::min<int>(spheres[i].B * r->intensity, 255);
                return c;
            }
        }

        r->z += 1;
        r->alive = r->z < 100;
    } while(r->alive);

    return c;
}

uint8_t check_impact(sphere *s, float x, float y, float z) {
    float xd = x - (s->x);
    float yd = y - (s->y);
    float zd = z - (s->z);

    float d = (xd*xd + yd*yd + zd*zd);
    float r = (s->radius) * (s->radius);

    return d <= r;
}
