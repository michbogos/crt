#include<stdio.h>
#include <omp.h>
#include<math.h>
#include<sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/time.h>     
#include <time.h>
#include"vec3.h"
#include"matrix.h"
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

#include "obj_loader.h"


int WIDTH =  512;
int HEIGHT =  512;
int SAMPLES =  100;
int TILESIZE = 64;

#include "material.h"
#include"util.h"

void render(struct World world, struct Camera cam, float* img, pcg32_random_t rng){
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
                c = vec3Add(c, linearScatter(rec, world, &rng, 10));
            }
            c = vec3Scale(c, 1.0f/SAMPLES);
            writePixelf(c.x, c.y, c.z, i, j, img, WIDTH, HEIGHT, 3);
        }
    }
}

void render_tiled(struct World world, struct Camera cam, float* img, pcg32_random_t rng){
    int progress = 0;
    #pragma omp parallel for
    for(int tiley = 0; tiley < HEIGHT/TILESIZE;tiley++){
        for(int tilex = 0; tilex < WIDTH/TILESIZE;tilex++){
            fprintf(stderr, "\r%d\\%d", progress, (HEIGHT/TILESIZE)*(WIDTH/TILESIZE));
            progress++;
            for(int j = 0; j < TILESIZE; j++){
                for(int i = 0; i < TILESIZE; i++){
                    ray r = getRay(cam, tilex*TILESIZE+i, tiley*TILESIZE+j, &rng);
                    struct vec3 c = (struct vec3){0, 0, 0};
                    ray tmp = r;
                    for(int sample = 0; sample < SAMPLES; sample++){
                        tmp.dir = vec3Add(r.dir, (struct vec3){intervalRandf(0.0f, 0.01, &rng), intervalRandf(0.0f, 0.01f, &rng), 0});
                        struct hitRecord rec = getHit(tmp, world);
                        c = vec3Add(c, linearScatter(rec, world, &rng, 10));
                    }
                    c = vec3Scale(c, 1.0f/SAMPLES);
                    writePixelf(c.x, c.y, c.z, tilex*TILESIZE+i, tiley*TILESIZE+j, img, WIDTH, HEIGHT, 3);
                }
            }
        }
    }
    for(int j = (HEIGHT/TILESIZE)*TILESIZE; j < HEIGHT; j++){
        for(int i = 0; i < WIDTH; i++){
            ray r = getRay(cam, i, j, &rng);
            struct vec3 c = (struct vec3){0, 0, 0};
            ray tmp = r;
            for(int sample = 0; sample < SAMPLES; sample++){
                tmp.dir = vec3Add(r.dir, (struct vec3){intervalRandf(0.0f, 0.01, &rng), intervalRandf(0.0f, 0.01f, &rng), 0});
                struct hitRecord rec = getHit(tmp, world);
                c = vec3Add(c, linearScatter(rec, world, &rng, 10));
            }
            c = vec3Scale(c, 1.0f/SAMPLES);
            writePixelf(c.x, c.y, c.z, i, j, img, WIDTH, HEIGHT, 3);
        }
    }

    for(int j = 0; j < (HEIGHT/TILESIZE)*TILESIZE; j++){
        for(int i = (WIDTH/TILESIZE)*TILESIZE; i < WIDTH; i++){
            ray r = getRay(cam, i, j, &rng);
            struct vec3 c = (struct vec3){0, 0, 0};
            ray tmp = r;
            for(int sample = 0; sample < SAMPLES; sample++){
                tmp.dir = vec3Add(r.dir, (struct vec3){intervalRandf(0.0f, 0.01, &rng), intervalRandf(0.0f, 0.01f, &rng), 0});
                struct hitRecord rec = getHit(tmp, world);
                c = vec3Add(c, linearScatter(rec, world, &rng, 10));
            }
            c = vec3Scale(c, 1.0f/SAMPLES);
            writePixelf(c.x, c.y, c.z, i, j, img, WIDTH, HEIGHT, 3);
        }
    }
}

int main(){

    stbi_set_flip_vertically_on_load(1);

    // int NUM_THREADS = get_nprocs();
    struct Camera cam = {.camera_up=(struct vec3){0, 1, 0}, .look_at=(struct vec3){0, 0, 0}, .pos=(struct vec3){5, 5, 5}, .fov=1.5};

    FILE *fptr;

    unsigned int seed = 42u;
    unsigned int initseq = 54u;

    // Open a file in read mode
    fptr = fopen("/dev/urandom", "rb");

    if(fptr != NULL){
        fread(&seed, 1, 4, fptr);
        fread(&initseq, 1, 4, fptr);

        fclose(fptr);
    }

    pcg32_random_t rng;
    pcg32_srandom_r(&rng, seed, initseq); // Constant seed 42 54

    initCamera(&cam, WIDTH, HEIGHT);

    struct World world;
    initWorld(&world);

    int envMap = texFromFile(&world, "environment.hdr");
    int lavender = texConst(&world, (struct vec3){0.7, 0.7, 1.0});
    int white = texConst(&world, (struct vec3){1.0, 1.0, 1.0});
    int normal = texFromFile(&world, "normal.jpg");
    int noise = texNoise(&world, 0.01f, unitRandf(&rng)*20000000);
    int checker = texChecker(&world, 0.2f, (struct vec3){0.0, 0.0, 0.0}, (struct vec3){1.0, 1.0, 1.0});
    int tiles = texFromFile(&world, "tiles.jpg");
    int texUv = texUV(&world);
    struct materialInfo mats[] = {
    (struct materialInfo){.max_bounces=10, .normal=-1, .texture=lavender, .type=METAL, .emissiveColor=(struct vec3){0, 0, 0}, .fuzz=0.0f, .ior=1.3},
    (struct materialInfo){.max_bounces=10, .normal=-1, .texture=lavender, .type=METAL, .emissiveColor=(struct vec3){1, 1, 1}, .fuzz=0.0f, .ior=1.3}};
    // (struct materialInfo){.normal = normal, .max_bounces=10, .texture=lavender, .emissiveColor=(struct vec3){2, 2, 2.0}},
    //                           (struct materialInfo){.max_bounces=10, .texture=&checker, .type=LAMBERT, .emissiveColor=(struct vec3){0, 0, 0}},
    //                           (struct materialInfo){.max_bounces=10, .type=DIELECTRIC, .fuzz=0.0f, .texture=lavender, .emissiveColor=(struct vec3){0, 0, 0}, .ior=1.333f},
    //                           (struct materialInfo){.max_bounces=10, .type=METAL, .fuzz=0.0f, .texture=lavender, .emissiveColor=(struct vec3){0, 0, 0}},
    //                           (struct materialInfo){.max_bounces=10, .texture=&checker, .type=LAMBERT, .emissiveColor=(struct vec3){0, 0, 0}},
    //                           (struct materialInfo){.normal = normal, .max_bounces=10, .texture=&lavender, .type=METAL, .fuzz=0.2f, .emissiveColor=(struct vec3){0, 0, 0}},
    //                           (struct materialInfo){.normal = normal, .max_bounces=10, .texture=&lavender, .type=DIELECTRIC, .ior=1.133f, .emissiveColor=(struct vec3){0, 0, 0}},
    //                           (struct materialInfo){.normal = normal, .max_bounces=10, .type=METAL, .fuzz=0.2f, .texture=white, .emissiveColor=(struct vec3){0, 0, 0}},
    //                           (struct materialInfo){.normal = normal, .max_bounces=10, .texture=&lavender, .emissiveColor=(struct vec3){1.4, 1.4, 2.0}},
    //                           (struct materialInfo){.normal = normal, .max_bounces=10, .texture=&lavender, .type=METAL, .fuzz=0.4f, .emissiveColor=(struct vec3){0, 0, 0}},
    //                           (struct materialInfo){.normal = normal, .max_bounces=10, .type=DIELECTRIC, .ior=1.0f, .emissiveColor=(struct vec3){0, 0, 0}, .texture=white}};
    
    world.materials = mats;
    world.envmap = envMap;

    struct vec3 t = (struct vec3){0, 0, 0};
    struct vec3 t2 = (struct vec3){0, 1, 0};

    float rotation[16] =    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    float translation[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    matRotation(rotation, (struct vec3){0.5, 0.5, 0.5});
    matTranslation(translation, (struct vec3){0, 0, 2});
    struct Mesh horse = addMesh(&world, "sphere.obj", 0, NULL);

    for(int i = 0; i < 6; i++){
        float* mat = calloc(16, sizeof(float));
        float* scale = calloc(16, sizeof(float));
        matRotation(rotation, (struct vec3){0, 1.0471975512*(i), 0});
        matScale(scale, (struct vec3){i%2 ? 0.8 : 1.2, i%2 ? 0.8 : 1.2, i%2 ? 0.8 : 1.2});
        matmul4x4(mat, translation, scale);
        matmul4x4(mat, rotation, mat);
        addMeshInstance(&world, &horse, mat);
    }

    addSphere(&world, &((struct Sphere){(struct vec3){0, 5, 0}, 0.1}), 1);

    // addSphere(&world, &((struct Sphere){(struct vec3){0, -5, 0}, 2}), 1);

    struct Hittable* objPtrs[world.objects.size];
    for(int i = 0; i < world.objects.size; i++){
        objPtrs[i] = &(world.objects.data[i]);
    }

    buildBvh(world.object_data, world.matrix_data, world.tree, objPtrs, world.objects.size);

    int node_count = countNodes(world.tree);

    struct LBvh* nodes = malloc(sizeof(struct LBvh)*node_count);
    struct AABB* boxes = malloc(sizeof(struct AABB)*node_count);

    int count = -1;
    buildLBvh(nodes, boxes, world.tree, &count);

    world.boxes = boxes;
    world.lbvh_nodes = nodes;

    for(int i = 0; i < node_count; i++){
        struct LBvh node = world.lbvh_nodes[i];
    }

    float* img = malloc(WIDTH*HEIGHT*3*sizeof(float));

    struct timeval tic, toc;
    double elapsed = 0.0;

    gettimeofday(&tic, NULL);
    render_tiled(world, cam, img, rng);
    gettimeofday(&toc, NULL);
    elapsed = (toc.tv_sec - tic.tv_sec) * 1000.0;      // sec to ms
    elapsed += (toc.tv_usec - tic.tv_usec) / 1000.0;   // us to ms
    printf("\nFinished in %lf seconds\n", elapsed/1000.0);
    stbi_write_hdr("img.hdr", WIDTH, HEIGHT, 3, img);
    // Implement resource free
    return 0;
}