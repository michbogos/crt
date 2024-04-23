#include<stdio.h>
#include<math.h>
#include"vec3.h"
#include"ray.h" 
#include"objects.h"
#include"pcg_basic.h"


#define WIDTH 512
#define HEIGHT 512
#define SAMPLES 20
#define DEPTH 10


// struct materialInfo lambert = {.attenuation = 0.5, .max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL, .fuz};
// struct materialInfo metal = {.attenuation = 0.5, .max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL};

struct vec3 centers[] = {(struct vec3){0,0,0}, (struct vec3){1, 1, 0}, (struct vec3){0.3, 0.3, -1}, (struct vec3){2, 2 ,0}, (struct vec3){-2, -2, 0}};
float radii[] = {0.5, 0.6, 0.3, 0.1, 0.1};
struct materialInfo mats[] = {(struct materialInfo){.attenuation = 0.5, .max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT},
                              (struct materialInfo){.attenuation = 0.5, .max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL, .fuzz=0.0f},
                              (struct materialInfo){.attenuation = 0.5, .max_bounces=10, .color={1.0, 1.0, 1.0}, .type=DIELECTRIC, .fuzz=0.0f, .ior=1.133f},
                              (struct materialInfo){.attenuation = 0.5, .max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT},
                              (struct materialInfo){.attenuation = 0.5, .max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT}};



pcg32_random_t rng;

//Maybe add sky as a seperate object and material
struct hitRecord getHit(ray r){
    int hit = 0;
    struct hitRecord rec;
    rec.t = 1000000.0f;
    for(int i = 0; i < 5; i++){
        struct hitRecord tmp;
        if(hitSphere(r, centers[i], radii[i], &tmp)){
            if(rec.t > tmp.t && tmp.t > 0.00001f){
                hit += 1;
                rec = tmp;
                rec.id = i;
            }
        }
    }
    rec.r = r;
    rec.mat = hit ? mats[rec.id] : mats[0];
    return rec;
}

#include "material.h"
#include"util.h"

int main(){
     pcg32_srandom_r(&rng, 42u, 54u); // Constant seed


    float aspectRatio = (float)WIDTH/(float)HEIGHT;
    float fov = 1.5f;
    float h = tanf(fov/2);
    struct vec3 camera_up = (struct vec3){0, 1, 0};
    struct vec3 look_at = (struct vec3){0, 0, 0.0f};
    struct vec3 center = (struct vec3){0, 0, -3};
    float focal_length = vec3Mag(vec3Sub(center, look_at));
    float viewport_height = 2*h*focal_length;
    float viewport_width = viewport_height*aspectRatio;
    struct vec3 w = vec3Unit(vec3Sub(center, look_at));
    struct vec3 u = vec3Unit(vec3Cross(camera_up, w));
    struct vec3 v = vec3Cross(w, u);

    struct vec3 viewport_u = vec3Scale(u, viewport_width);
    struct vec3 viewport_v = vec3Scale(v, viewport_height);
    struct vec3 du = vec3Scale(viewport_u, 1.0f/WIDTH);
    struct vec3 dv = vec3Scale(viewport_v, 1.0f/HEIGHT);
    struct vec3 top_left = vec3Sub(vec3Sub(vec3Sub(center, vec3Scale(w, -focal_length)), vec3Scale(viewport_u, 0.5)), vec3Scale(viewport_v, 0.5));
    printf("P3\n%d %d\n255\n", WIDTH, HEIGHT);
    for(int  j = 0 ; j < HEIGHT; j++){
        for(int i = 0 ;i < WIDTH; i++){
            struct vec3 dest = vec3Add(vec3Add(top_left, vec3Scale(du, i)), vec3Scale(dv, j));
            // float y = ((float)j/(float)(HEIGHT))*h*2;
            // float z = 0;
            ray r;
            r.origin = center;
            r.dir = vec3Sub(center, dest);//vec3Sub((struct vec3){x, y, 0}, r.origin);
            struct vec3 c = (struct vec3){0, 0, 0};
            for(int sample = 0 ; sample < SAMPLES; sample++){
                ray tmp = r;
                tmp.dir = vec3Add(tmp.dir, (struct vec3){intervalRandf(0.0f, 0.001f, &rng), intervalRandf(0.0f, 0.001f, &rng), 0});
                c = vec3Add(c, scatter(getHit(tmp), &rng, 0));
            }
            c = vec3Scale(c, 1.0f/SAMPLES);
            writeColor(c.x, c.y, c.z);
            //writeColor(r, g, b);
        }
    }
    return 0;
}