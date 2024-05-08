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


int WIDTH =  512;
int HEIGHT =  512;
int SAMPLES =  18;

#include "material.h"
#include"util.h"

int main(){

    struct Texture tex;
    struct Camera cam = {.cmaera_up=(struct vec3){0, 1, 0}, .look_at=(struct vec3){0, 1, 0}, .pos=(struct vec3){-2, 2, -3}, .fov=1.5};

    FILE *fptr;

    // Open a file in read mode
    fptr = fopen("/dev/urandom", "rb");

    unsigned int seed;
    unsigned int initseq;
    fread(&seed, 1, 4, fptr);
    fread(&initseq, 1, 4, fptr);

    fclose(fptr);

    pcg32_random_t rng;
    pcg32_srandom_r(&rng, seed, initseq); // Constant seed 42 54

    initCamera(&cam, WIDTH, HEIGHT);

    tex = texFromFile("2k_earth_daymap.jpg");

    struct materialInfo mats[] = {(struct materialInfo){.max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL, .fuzz=0.2f, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.9, 0.9}, .type=DIELECTRIC, .ior=1.133f, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.2, 0.7}, .type=METAL, .fuzz=0.2f, .texture=&tex, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.max_bounces=10, .color={0, 0, 0}, .emissiveColor=(struct vec3){1.4, 1.4, 2.0}},
                              (struct materialInfo){.max_bounces=10, .color={0.9, 0.9, 0.1}, .type=METAL, .fuzz=0.4f, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.9, 0.9}, .type=DIELECTRIC, .ior=1.133f, .emissiveColor=(struct vec3){0, 0, 0}, .texture=&tex}};

    struct World world = {.materials=mats};

    initWorld(&world);

    struct Quad* q = malloc(sizeof(struct Quad));
    q->p = (struct vec3){-5, 0, 5};
    q->u = (struct vec3){10, 0, 0};
    q->v = (struct vec3){0, 0, -10};
    addQuad(&world, q, 1);

    struct Sphere* s = malloc(sizeof(struct Sphere));
    s->center = (struct vec3){0,0.5f,0};
    s->radius = 0.5f;
    addSphere(&world, s, 0);
    
    struct Sphere* s2 = malloc(sizeof(struct Sphere));
    s2->center = (struct vec3){0,3.0f,0};
    s2->radius = 1.0f;
    addSphere(&world, s2, 4);

    for(int i = 0; i < 125; i++){
        struct Sphere* s = malloc(sizeof(struct Sphere));
        s->center = (struct vec3){unitRandf(&rng)*4-2.0f,unitRandf(&rng)*0.05+0.15, unitRandf(&rng)*4-2.0f};
        s->radius = unitRandf(&rng)*0.15;
        addSphere(&world, s, rand()%7);
    }

    struct Triangle* tri = malloc(sizeof(struct Triangle));
    tri->a = (struct vec3){1, 1.1, 1};
    tri->b = (struct vec3){-1, 1.1, 0};
    tri->c = (struct vec3){0, 1.1, -1};

    addTri(&world, tri, 1);

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