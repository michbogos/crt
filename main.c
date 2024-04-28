#include<stdio.h>
#include <omp.h>
#include<math.h>
#include"vec3.h"
#include"ray.h" 
#include"objects.h"
#include"camera.h"
#include"pcg_basic.h"
#include"world.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


int WIDTH =  1024;
int HEIGHT =  1024;
int SAMPLES =  100;
int DEPTH =  10;

struct vec3 centers[] = {(struct vec3){0,0,-0.2}, (struct vec3){1, 1, 2}, (struct vec3){0.3, 0.3, -1}, (struct vec3){2, 2 ,0}, (struct vec3){-2, -2, 0}};
float radii[] = {0.5, 0.6, 0.3, 0.1, 2};
struct materialInfo mats[] = {(struct materialInfo){.max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT},
                              (struct materialInfo){.max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL, .fuzz=0.2f},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.9, 0.9}, .type=DIELECTRIC, .ior=1.133f},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.2, 0.7}, .type=METAL, .fuzz=0.2f}};

struct World world = {.materials=mats, .pos=centers, .radii=radii};

struct Camera cam = {.cmaera_up=(struct vec3){0, 1, 0}, .look_at=(struct vec3){0, 0, 0}, .pos=(struct vec3){0, 0, -7}, .fov=1.5};



pcg32_random_t rng;

#include "material.h"
#include"util.h"

int main(){
    pcg32_srandom_r(&rng, 42u, 54u); // Constant seed

    initCamera(&cam, WIDTH, HEIGHT);

    //Load environment map
    int env_w = 0;
    int env_h = 0;
    int channels = 0;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* env_map = stbi_load("environment.png", &env_w, &env_h, &channels, 3);

    unsigned char* img = malloc(WIDTH*HEIGHT*3);

    printf("P3\n%d %d\n255\n", WIDTH, HEIGHT);
    int progress = 0;
    #pragma omp parallel for
    for(int  j = 0 ; j < HEIGHT; j++){
        fprintf(stderr, "\r%d\\%d", progress, HEIGHT);
        progress ++;
        for(int i = 0 ;i < WIDTH; i++){
            ray r = getRay(cam, i, j, &rng);
            struct vec3 c = (struct vec3){0, 0, 0};
            ray tmp = r;
            for(int sample = 0 ; sample < SAMPLES; sample++){
                tmp.dir = vec3Add(r.dir, (struct vec3){intervalRandf(0.0f, 0.01, &rng), intervalRandf(0.0f, 0.01f, &rng), 0});
                c = vec3Add(c, scatter(getHit(tmp, world), world, &rng, 0, env_map, env_w, env_h));
            }
            c = vec3Scale(c, 1.0f/SAMPLES);
            writePixel(c.x, c.y, c.z, i, j, img, WIDTH, HEIGHT, 3);
        }
    }
    stbi_write_png("img.png", WIDTH, HEIGHT, 3, img, sizeof(unsigned char)*WIDTH*3);
    stbi_image_free(env_map);
    return 0;
}