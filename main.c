#include<stdio.h>
#include <omp.h>
#include<math.h>
#include<sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
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

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "obj_loader.h"


int WIDTH =  512;
int HEIGHT =  512;
int SAMPLES =  18;

#include "material.h"
#include"util.h"

static void get_file_data(void* ctx, const char* filename, const int is_mtl,
                          const char* obj_filename, char** data, size_t* len) {
  // NOTE: If you allocate the buffer with malloc(),
  // You can define your own memory management struct and pass it through `ctx`
  // to store the pointer and free memories at clean up stage(when you quit an
  // app)
  // This example uses mmap(), so no free() required.
  (void)ctx;
  if (!filename) {
    fprintf(stderr, "null filename\n");
    (*data) = NULL;
    (*len) = 0;
    return;
  }

    size_t data_len = 0;

    FILE* file;

    file = fopen(obj_filename, "r");

    assert(file != NULL);

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);  /* same as rewind(f); */

    *data = malloc(fsize + 1);
    fread(*data, fsize, 1, file);
    fclose(file);
    (*len) = fsize;
}

int main(){

    struct Texture tex;
    struct Camera cam = {.cmaera_up=(struct vec3){0, 1, 0}, .look_at=(struct vec3){0, 1, 0}, .pos=(struct vec3){-2, 2, 5}, .fov=1.5};

    FILE *fptr;

    // Open a file in read mode
    fptr = fopen("/dev/urandom", "rb");

    unsigned int seed;
    unsigned int initseq;
    fread(&seed, 1, 4, fptr);
    fread(&initseq, 1, 4, fptr);

    fclose(fptr);

    pcg32_random_t rng;
    pcg32_srandom_r(&rng, 42u, 54u); // Constant seed 42 54

    initCamera(&cam, WIDTH, HEIGHT);

    tex = texFromFile("2k_earth_daymap.jpg");
    struct Texture lavender = texConst((struct vec3){0.7, 0.7, 1.0});
    struct Texture normal = texFromFile("normal.jpg");
    struct Texture noise = texNoise(0.01f, unitRandf(&rng)*20000000);
    struct Texture checker = texChecker(0.05f, (struct vec3){0.0, 0.0, 0.0}, (struct vec3){1.0, 1.0, 1.0});
    struct materialInfo mats[] = {(struct materialInfo){.max_bounces=10, .texture=&noise, .type=LAMBERT, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.max_bounces=10, .texture=&checker, .type=LAMBERT, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.normal = &normal, .max_bounces=10, .texture=&lavender, .type=METAL, .fuzz=0.2f, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.normal = &normal, .max_bounces=10, .texture=&lavender, .type=DIELECTRIC, .ior=1.133f, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.normal = &normal, .max_bounces=10, .type=METAL, .fuzz=0.2f, .texture=&tex, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.normal = &normal, .max_bounces=10, .texture=&lavender, .emissiveColor=(struct vec3){1.4, 1.4, 2.0}},
                              (struct materialInfo){.normal = &normal, .max_bounces=10, .texture=&lavender, .type=METAL, .fuzz=0.4f, .emissiveColor=(struct vec3){0, 0, 0}},
                              (struct materialInfo){.normal = &normal, .max_bounces=10, .type=DIELECTRIC, .ior=1.133f, .emissiveColor=(struct vec3){0, 0, 0}, .texture=&tex}};

    struct World world = {.materials=mats};

    initWorld(&world);

    // struct Quad* q = malloc(sizeof(struct Quad));
    // q->p = (struct vec3){-5, 0, 5};
    // q->u = (struct vec3){10, 0, 0};
    // q->v = (struct vec3){0, 0, -10};
    // addQuad(&world, q, 1);

    // struct Sphere* s = malloc(sizeof(struct Sphere));
    // s->center = (struct vec3){0,0.5f,0};
    // s->radius = 0.5f;
    // addSphere(&world, s, 0);
    
    // struct Sphere* s2 = malloc(sizeof(struct Sphere));
    // s2->center = (struct vec3){0,3.0f,0};
    // s2->radius = 1.5f;
    // addSphere(&world, s2, 4);

    // for(int i = 0; i < 125; i++){
    //     struct Sphere* s = malloc(sizeof(struct Sphere));
    //     s->center = (struct vec3){unitRandf(&rng)*4-2.0f,unitRandf(&rng)*0.05+0.15, unitRandf(&rng)*4-2.0f};
    //     s->radius = unitRandf(&rng)*0.15;
    //     addSphere(&world, s, rand()%8);
    // }

    // struct Triangle* tri = malloc(sizeof(struct Triangle));
    // tri->a = (struct vec3){1, 1.1, 1};
    // tri->b = (struct vec3){-1, 1.1, 0};
    // tri->c = (struct vec3){0, 1.1, -1};

    // addTri(&world, tri, 3);

    //Load obj file

    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes;
    tinyobj_material_t* materials = NULL;
    size_t num_materials;

    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
    int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, "suzanne.obj", get_file_data, NULL, flags);

    int num_triangles = attrib.num_face_num_verts;
    int face_offset = 0;

    for (int i = 0; i < attrib.num_face_num_verts; i++) {
      assert(attrib.face_num_verts[i] % 3 ==
             0); /* assume all triangle faces. */
      for (int f = 0; f < (size_t)attrib.face_num_verts[i] / 3; f++) {
        size_t k;
        // float v[3][3];
        // float n[3][3];
        // float c[3];
        float len2;

        tinyobj_vertex_index_t idx0 = attrib.faces[face_offset + 3 * f + 0];
        tinyobj_vertex_index_t idx1 = attrib.faces[face_offset + 3 * f + 1];
        tinyobj_vertex_index_t idx2 = attrib.faces[face_offset + 3 * f + 2];

        int f0 = idx0.v_idx;
        int f1 = idx1.v_idx;
        int f2 = idx2.v_idx;
        assert(f0 >= 0);
        assert(f1 >= 0);
        assert(f2 >= 0);

        struct Triangle* tri = malloc(sizeof(struct Triangle));
        tri->a = (struct vec3){attrib.vertices[3 * (size_t)f0 + 0],
                              attrib.vertices[3 * (size_t)f0 + 1],
                              attrib.vertices[3 * (size_t)f0 + 2]};
        
        tri->b = (struct vec3){attrib.vertices[3 * (size_t)f1 + 0],
                              attrib.vertices[3 * (size_t)f1 + 1],
                              attrib.vertices[3 * (size_t)f1 + 2]};
        
        tri->c = (struct vec3){attrib.vertices[3 * (size_t)f2 + 0],
                              attrib.vertices[3 * (size_t)f2 + 1],
                              attrib.vertices[3 * (size_t)f2 + 2]};
        
        addTri(&world, tri, 0);
        }
        face_offset += (size_t)attrib.face_num_verts[i];
    }

    struct Hittable* objPtrs[world.objects.size];
    for(int i = 0; i < world.objects.size; i++){
        objPtrs[i] = &(world.objects.data[i]);
    }

    struct Bvh tree;

    buildBvh(&tree, objPtrs, world.objects.size);

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