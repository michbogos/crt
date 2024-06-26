#ifndef WORLD_H
#define WORLD_H

#include "objects.h"
#include "bvh.h"
#include "vector.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "obj_loader.h"

#include "util.h"
#include <stdlib.h>
#include <string.h>

struct World{
    struct materialInfo* materials;
    struct Vector objects;
    struct Bvh* tree;
    struct LBvh* lbvh_nodes;
    struct AABB* boxes;
    struct Texture* textures;
    int envmap;
    int num_textures;
    char* object_data;
    int object_data_size;
    float* matrix_data;
    int matrix_data_size;
    float* texture_data;
    int texture_data_size;
};

//Maybe add sky as a seperate object and material
struct hitRecord getHit(ray r, struct World world){
    char* base = world.object_data;
    int hit = 0;
    struct hitRecord rec;
    rec.t = 1000000.0f;
    rec.r = r;
    struct Vector hittables;
    vectorInit(&hittables);
    traverseLBvh(world.objects.data, &hittables, world.lbvh_nodes,world.boxes, r);
    //traverseBvh(&hittables, world.tree, r);
    for(int i = 0; i < hittables.size; i++){
        struct hitRecord tmp;
        //Transformed ray
        ray tmpr = r;
        struct vec3 point = vec3Add(tmpr.origin, tmpr.dir);
        tmpr.origin = hittables.data[i].matrix_offset != -1 ? vec3matmul(tmpr.origin, world.matrix_data+hittables.data[i].inverse_offset) : tmpr.origin;
        point = hittables.data[i].matrix_offset != -1 ? vec3matmul(point, world.matrix_data+hittables.data[i].inverse_offset) : point;
        tmpr.dir = vec3Sub(point, tmpr.origin);
        switch (hittables.data[i].type)
        {
            case SPHERE:
                struct Sphere s = *((struct Sphere*)(base+hittables.data[i].offset));
                if(hitSphere(tmpr, s, &tmp)){
                    if(rec.t > tmp.t && tmp.t > 0.00001f){
                        hit += 1;
                        rec = tmp;
                        rec.mat = world.materials[hittables.data[i].matIndex];
                        rec.r = r;
                    }
                }
                break;
            case QUAD:
                struct Quad q = *((struct Quad*)(base+hittables.data[i].offset));
                if(hitQuad(r, q, &tmp)){
                    if(rec.t > tmp.t && tmp.t > 0.00001f){
                        hit += 1;
                        rec = tmp;
                        rec.mat = world.materials[hittables.data[i].matIndex];
                    }
                }
                break;
            case TRI:
                struct Triangle tri = *((struct Triangle*)(base+hittables.data[i].offset));
                //Think of ray as two points
                if(hitTri(tmpr, tri, &tmp)){
                    if(rec.t > tmp.t && tmp.t > 0.00001f){
                        hit += 1;
                        rec = tmp;
                        rec.mat = world.materials[hittables.data[i].matIndex];
                        rec.normal = world.matrix_data+hittables.data[i].matrix_offset != -1 ? vec3Unit(vec3Sub(vec3matmul(rec.normal, world.matrix_data+hittables.data[i].matrix_offset), vec3matmul((struct vec3){0, 0, 0}, world.matrix_data+hittables.data[i].matrix_offset))) : rec.normal;
                        rec.r = r;
                    }
                }
                break;
        default:
            printf("Default case\n");
            break;
        }
    }
    rec.mat = hit? rec.mat : world.materials[0];
    free(hittables.data);
    assert(rec.r.dir.x != NAN);
    return rec;
}

void initWorld(struct World * w){
    vectorInit(&(w->objects));
    w->tree = malloc(sizeof(struct Bvh));
    w->object_data = malloc(1024*1024*128);
    w->object_data_size = 0;
    w->matrix_data = malloc(1024*1024*8);
    w->matrix_data_size = 0;
    w->texture_data = malloc(1024*1024*256);
    w->texture_data_size = 0;
    w->textures = malloc(32*sizeof(struct Texture));
    w->num_textures = 0;
}

//Transform not impemented possible out of bounds memory access
void addSphere(struct World* world, struct Sphere* s, int matIndex){
    memcpy(world->object_data+world->object_data_size, s, sizeof(struct Sphere));
    vectorPush(&(world->objects), (struct Hittable){.type=SPHERE, .offset=world->object_data_size, .matIndex=matIndex, .id=world->objects.size});
    world->object_data_size += sizeof(struct Sphere);
}

//Transform not implemented possible out of bounds memory access
void addQuad(struct World* world, struct Quad* quad, int matIndex){
    quad->n = vec3Cross(quad->u, quad->v);
    quad->normal = vec3Unit(quad->n);
    quad->D = vec3Dot(quad->normal, quad->p);
    quad->w = vec3Scale(quad->n, 1.0f/vec3Dot(quad->n,quad->n));

    memcpy(world->object_data+world->object_data_size, quad, sizeof(struct Quad));
    vectorPush(&(world->objects), (struct Hittable){.type=QUAD, .offset=world->object_data_size, .matIndex=matIndex, .id=world->objects.size});
    world->object_data_size += sizeof(struct Quad);
}

void addTri(struct World* world, struct Triangle* tri, int matIndex, float* transform){
    memcpy(world->object_data+world->object_data_size, tri, sizeof(struct Triangle));
    memcpy(world->matrix_data+world->matrix_data_size, transform, sizeof(float)*16);
    vectorPush(&(world->objects), (struct Hittable){.type=TRI, .offset=world->object_data_size, .matIndex=matIndex, .matrix_offset=world->matrix_data_size, .id=world->objects.size});
    world->object_data_size += sizeof(struct Triangle);
    world->matrix_data_size += 16;
}

struct Mesh addMesh(struct World* world, const char* path, int matIndex, float* transform){
    struct Mesh m;
    m.index = world->objects.size;
    m.size = 0;
    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes;
    tinyobj_material_t* materials = NULL;
    size_t num_materials;

    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
    int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, path, get_file_data, NULL, flags);

    int num_triangles = attrib.num_face_num_verts;
    int face_offset = 0;

    float* inverse_transform = NULL;

    if(transform != NULL){
        inverse_transform = calloc(16, sizeof(float));
        for(int i = 0; i < 16; i++){
            inverse_transform[i] = transform[i];
        }

        if(!matInvert(inverse_transform)){
            assert(-1 && "matrix not invertible");
        }
    }
    else{
        transform = calloc(16, sizeof(float));
        inverse_transform = calloc(16, sizeof(float));
        matScale(transform, (struct vec3){1, 1, 1});
        matScale(inverse_transform, (struct vec3){1, 1, 1});
    }

    // printf("Tri count: %d\n", attrib.num_texcoords);

    for (int i = 0; i < attrib.num_face_num_verts; i++) {
      assert(attrib.face_num_verts[i] % 3 ==
             0); /* assume all triangle faces. */
      for (int f = 0; f < (size_t)attrib.face_num_verts[i] / 3; f++) {
        size_t k;
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
        
        f0 = idx0.vn_idx;
        f1 = idx1.vn_idx;
        f2 = idx2.vn_idx;
        
        tri->norma = (struct vec3){attrib.normals[3*(size_t)f0+2], attrib.normals[3*(size_t)f0+1], attrib.normals[3*(size_t)f0+0]};
        tri->normb = (struct vec3){attrib.normals[3*(size_t)f1+2], attrib.normals[3*(size_t)f1+1], attrib.normals[3*(size_t)f1+0]};
        tri->normc = (struct vec3){attrib.normals[3*(size_t)f2+2], attrib.normals[3*(size_t)f2+1], attrib.normals[3*(size_t)f2+0]};

        f0 = idx0.vt_idx;
        f1 = idx1.vt_idx;
        f2 = idx2.vt_idx;

        tri->uva = (struct vec3){attrib.texcoords[2*(size_t)f0+0], attrib.texcoords[2*(size_t)f0+1], 0};
        tri->uvb = (struct vec3){attrib.texcoords[2*(size_t)f1+0], attrib.texcoords[2*(size_t)f1+1], 0};
        tri->uvc = (struct vec3){attrib.texcoords[2*(size_t)f2+0], attrib.texcoords[2*(size_t)f2+1], 0};


        memcpy(world->object_data+world->object_data_size, tri, sizeof(struct Triangle));
        memcpy(world->matrix_data+world->matrix_data_size, transform, sizeof(float)*16);
        memcpy(world->matrix_data+world->matrix_data_size+16, inverse_transform, sizeof(float)*16);
        vectorPush(&(world->objects), (struct Hittable){.type=TRI, .offset=world->object_data_size, .matIndex=matIndex, .matrix_offset=world->matrix_data_size, .inverse_offset=world->matrix_data_size+16, .id=world->objects.size});
        world->object_data_size += sizeof(struct Triangle);
        world->matrix_data_size += 32;
        m.size++;
        }
        face_offset += (size_t)attrib.face_num_verts[i];
    }
    m.matIdx = matIndex;
    return m;
}

void addMeshInstance(struct World* world, struct Mesh* mesh, float* transform){
    float* inverse_transform = calloc(16, sizeof(float));
    for(int i = 0; i < 16; i++){
        inverse_transform[i] = transform[i];
    }

    if(!matInvert(inverse_transform)){
        assert(-1 && "matrix not invertible");
    }

    memcpy(world->matrix_data+world->matrix_data_size, transform, sizeof(float)*16);
    memcpy(world->matrix_data+world->matrix_data_size+16, inverse_transform, sizeof(float)*16);
    for(int i = 0; i < mesh->size; i++){
        vectorPush(&(world->objects), (struct Hittable){.type=TRI, .offset=world->objects.data[mesh->index+i].offset, .matIndex=world->objects.data[mesh->index+i].matIndex, .matrix_offset=world->matrix_data_size, .inverse_offset=world->matrix_data_size+16, .id=world->objects.size});
    }
    world->matrix_data_size += 32;
}

int texFromFile(struct World* world, const char * filename){
    struct Texture tex;
    int x;
    int y;
    int ch;
    float* data = stbi_loadf(filename, &x, &y, &ch, 3);
    memcpy(world->texture_data+world->texture_data_size, data, x*y*ch*sizeof(float));
    tex.offset = world->texture_data_size;
    world->texture_data_size += x*y*ch;
    tex.x = x;
    tex.y = y;
    tex.channels = ch;
    tex.type = TEXTURE_2D;
    world->num_textures += 1;
    world->textures[world->num_textures-1] = tex;
    return world->num_textures-1;
}

int texConst(struct World* world, struct vec3 color){
    struct Texture tex;
    tex.type = TEXTURE_CONST;
    float data[3];
    tex.offset = world->texture_data_size;
    data[0] = color.x;
    data[1] = color.y;
    data[2] = color.z;
    memcpy(world->texture_data+world->texture_data_size, data, 3*sizeof(float));
    world->texture_data_size += 3;
    world->num_textures += 1;
    world->textures[world->num_textures-1] = tex;
    return world->num_textures-1;
}

// struct Texture texPerlin(float scale, float seed){
//     struct Texture tex;
//     tex.type = TEXTURE_PERLIN;
//     tex.data = malloc(sizeof(float));
//     tex.data[0] = seed;
//     tex.scale = scale;
//     return tex;
// }

int texNoise(struct World* world, float scale, float seed){
    struct Texture tex;
    tex.type = TEXTURE_NOISE;
    float data[1];
    tex.offset = world->texture_data_size;
    data[0] = seed;
    memcpy(world->texture_data+world->texture_data_size, data, 1*sizeof(float));
    world->texture_data_size += 1;
    tex.scale = scale;
    world->num_textures += 1;
    world->textures[world->num_textures-1] = tex;
    return world->num_textures-1;
}

int texChecker(struct World* world, float scale, struct vec3 color1, struct vec3 color2){
    struct Texture tex;
    tex.type = TEXTURE_CHECKER;
    float data[6];
    data[0] = color1.x;
    data[1] = color1.y;
    data[2] = color1.z;
    data[3] = color2.x;
    data[4] = color2.y;
    data[5] = color2.z;
    tex.offset = world->texture_data_size;
    memcpy(world->texture_data+world->texture_data_size, data, 6*sizeof(float));
    world->texture_data_size += 6;
    tex.scale = scale;
    world->num_textures += 1;
    world->textures[world->num_textures-1] = tex;
    return world->num_textures-1;
}

int texUV(struct World* world){
    struct Texture tex;
    tex.type = TEXTURE_UV;
    world->num_textures += 1;
    world->textures[world->num_textures-1] = tex;
    return world->num_textures-1;
}

#endif