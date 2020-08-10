#include <rays.h>
#define PI 3.141592653589793238462643383279
#define cap_4pi(x) if (x > 4*PI) { x = 0; }

color* ray_marching(ray *r, sphere *spheres, int n_spheres, color *c) {
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

        s[i].R = rand() % 255;
        s[i].G = rand() % 255;
        s[i].B = rand() % 255;

        s[i].radius = s[i].z - s[i].z*0.1;
        s[i].pos_x = s[i].x;
        s[i].pos_y = s[i].y;
        s[i].pos_z = s[i].z;

        s[i].curr_i = 0.;
        s[i].curr_z = 0.;

        s[i].scale_i = (rand() % 200);
        s[i].scale_z = (rand() % 200);

    }

    return s;
}

void move_spheres(sphere *spheres, int n_spheres) {
  for (int i = 0; i < n_spheres; i++) {
    spheres[i].x = spheres[i].pos_x + spheres[i].scale_i * cos(spheres[i].curr_i);
    spheres[i].y = spheres[i].pos_y + spheres[i].scale_i * sin(spheres[i].curr_i);
    spheres[i].z = spheres[i].pos_z + spheres[i].scale_i * sin(spheres[i].curr_z);

    spheres[i].curr_i += 0.01;
    spheres[i].curr_z += 0.01;
    cap_4pi(spheres[i].curr_z);
    cap_4pi(spheres[i].curr_z);
  }
}
