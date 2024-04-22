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

struct vec3 centers[] = {(struct vec3){0,0,3}, (struct vec3){1, 1.7, 2}};
float radii[] = {1, 1};
struct materialInfo mats[] = {(struct materialInfo){.attenuation = 0.5, .max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT},
                              (struct materialInfo){.attenuation = 0.5, .max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL, .fuzz=0.0f}};



pcg32_random_t rng;

//Maybe add sky as a seperate object and material
struct hitRecord getHit(ray r){
    int hit = 0;
    struct hitRecord rec;
    rec.t = 1000000.0f;
    for(int i = 0; i < 2; i++){
        struct hitRecord tmp;
        if(hitSphere(r, centers[i], radii[i], &tmp)){
            hit += 1;
            if(rec.t > tmp.t && tmp.t > 0.00001f){
                rec = tmp;
                rec.id = i;
            }
        }
    }
    rec.r = r;
    rec.mat = mats[rec.id];
    return rec;
}

#include "material.h"
#include"util.h"

int main(){
     pcg32_srandom_r(&rng, 42u, 54u); // Constant seed

    float aspectRatio = (float)WIDTH/(float)HEIGHT;
    printf("P3\n%d %d\n255\n", WIDTH, HEIGHT);
    for(int  j = 0 ; j < HEIGHT; j++){
        for(int i = 0 ;i < WIDTH; i++){
            float x = ((float)i/(float)(WIDTH))*aspectRatio;
            float y = ((float)j/(float)(HEIGHT));
            float z = 0;
            ray r;
            r.origin = (struct vec3){0.5, 0.5, -0.5};
            r.dir = vec3Sub((struct vec3){x, y, 0}, r.origin);
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