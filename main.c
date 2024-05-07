#include<stdio.h>
#include <omp.h>
#include<math.h>
#include"vec3.h"
#include"ray.h" 
#include"objects.h"
#include"camera.h"
#include"pcg_basic.h"
#include"world.h"
#include"texture.h"
#include"bvh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


int WIDTH =  1024;
int HEIGHT =  1024;
int SAMPLES =  10;

#include "material.h"
#include"util.h"

int main(){

    struct Texture tex;
    struct Camera cam = {.cmaera_up=(struct vec3){0, 1, 0}, .look_at=(struct vec3){0, 0, 0}, .pos=(struct vec3){0, 2, -3}, .fov=1.5};



    pcg32_random_t rng;
    pcg32_srandom_r(&rng, 42u, 54u); // Constant seed

    initCamera(&cam, WIDTH, HEIGHT);

    tex = texFromFile("2k_earth_daymap.jpg");

    struct materialInfo mats[] = {(struct materialInfo){.max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT},
                              (struct materialInfo){.max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL, .fuzz=0.2f},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.9, 0.9}, .type=DIELECTRIC, .ior=1.133f},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.2, 0.7}, .type=METAL, .fuzz=0.2f, .texture=&tex}};

    struct World world = {.materials=mats};

    initWorld(&world);

    for(float x = -1; x < 1; x+=0.3f){
        for(float y = -1; y < 1; y+=0.3f){
            for(float z = -1; z < 1; z+=0.3f){
                struct Sphere* s = malloc(sizeof(struct Sphere));
                s->center = (struct vec3){x,y,z};
                s->radius = 0.15f;
                addSphere(&world, s, rand()%4);
            }
        }
    }

    struct Hittable* objPtrs[world.size];
    for(int i = 0; i < world.size; i++){
        objPtrs[i] = &(world.objects[i]);
    }

    struct Bvh tree;

    buildBvh(&tree, objPtrs, world.size);

    world.tree = &tree;

    //Load environment map
    int env_w = 0;
    int env_h = 0;
    int channels = 0;
    stbi_set_flip_vertically_on_load(1);
    float* env_map = stbi_loadf("environment.hdr", &env_w, &env_h, &channels, 3);
    float* img = malloc(WIDTH*HEIGHT*3*sizeof(float));

    // printf("P3\n%d %d\n255\n", WIDTH, HEIGHT);
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
                struct hitRecord rec = getHit(tmp, world);
                c = vec3Add(c, scatter(rec, world, &rng, 0, env_map, env_w, env_h));
            }
            c = vec3Scale(c, 1.0f/SAMPLES);
            writePixelf(c.x, c.y, c.z, i, j, img, WIDTH, HEIGHT, 3);
        }
    }
    stbi_write_hdr("img.hdr", WIDTH, HEIGHT, 3, img);
    stbi_image_free(env_map);
    // Implement resource free
    return 0;
}