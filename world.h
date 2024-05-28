#ifndef WORLD_H
#define WORLD_H

#include "objects.h"
#include "bvh.h"
#include "vector.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "obj_loader.h"

#include "util.h"
#include "arena.h"
#include <stdlib.h>
#include <string.h>

struct World{
    struct materialInfo* materials;
    struct Vector objects;
    struct Bvh* tree;
    struct LBvh* lbvh_nodes;
    struct AABB* boxes;
    struct Texture* envMap;
    char* object_data;
    int object_data_size; 
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
        tmpr.origin = hittables.data[i].transform_matrix != NULL ? vec3matmul(tmpr.origin, hittables.data[i].inverse_matrix) : tmpr.origin;
        point = hittables.data[i].transform_matrix != NULL ? vec3matmul(point, hittables.data[i].inverse_matrix) : point;
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
                        rec.normal = hittables.data[i].transform_matrix != NULL ? vec3Unit(vec3Sub(vec3matmul(rec.normal, hittables.data[i].transform_matrix), vec3matmul((struct vec3){0, 0, 0}, hittables.data[i].transform_matrix))) : rec.normal;
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
    return rec;
}

void initWorld(struct World * w, struct Texture* envMap){
    vectorInit(&(w->objects));
    w->tree = malloc(sizeof(struct Bvh));
    w->envMap = envMap;
    w->object_data = malloc(1024*1024*128);
    w->object_data_size = 0;
}

void addSphere(struct World* world, struct Sphere* s, int matIndex){
    memcpy(world->object_data+world->object_data_size, s, sizeof(struct Sphere));
    vectorPush(&(world->objects), (struct Hittable){.type=SPHERE, .offset=world->object_data_size, .matIndex=matIndex, .id=world->objects.size});
    world->object_data_size += sizeof(struct Sphere);
}

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
    vectorPush(&(world->objects), (struct Hittable){.type=TRI, .offset=world->object_data_size, .matIndex=matIndex, .transform_matrix=transform, .id=world->objects.size});
    world->object_data_size += sizeof(struct Triangle);
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
        float* inverse_transform = calloc(16, sizeof(float));
        for(int i = 0; i < 16; i++){
            inverse_transform[i] = transform[i];
        }

        if(!matInvert(inverse_transform)){
            assert(-1 && "matrix not invertible");
        }
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
        vectorPush(&(world->objects), (struct Hittable){.type=TRI, .offset=world->object_data_size, .matIndex=matIndex, .transform_matrix=transform, .inverse_matrix=inverse_transform, .id=world->objects.size});
        world->object_data_size += sizeof(struct Triangle);
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
    for(int i = 0; i < mesh->size; i++){
        vectorPush(&(world->objects), (struct Hittable){.type=TRI, .offset=(struct Triangle*)(world->objects.data[mesh->index+i].offset), .matIndex=world->objects.data[mesh->index+i].matIndex, .transform_matrix=transform, .inverse_matrix=inverse_transform, .id=world->objects.size});
    }
}

#endif