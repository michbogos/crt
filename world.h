#ifndef WORLD_H
#define WORLD_H

#include "objects.h"
#include "bvh.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

struct World{
    struct materialInfo* materials;
    struct Vector objects;
    struct Bvh* tree;
};

//Maybe add sky as a seperate object and material
struct hitRecord getHit(ray r, struct World world){
    int hit = 0;
    struct hitRecord rec;
    rec.t = 1000000.0f;
    rec.r = r;
    struct Bvh* bvh = world.tree;
    struct Vector hittables;
    vectorInit(&hittables);
    traverseBvh(&hittables, bvh, r);
    for(int i = 0; i < hittables.size; i++){
        struct hitRecord tmp;
        switch (hittables.data[i].type)
        {
            case SPHERE:
                struct Sphere s = *((struct Sphere*)hittables.data[i].data);
                if(hitSphere(r, s, &tmp)){
                    if(rec.t > tmp.t && tmp.t > 0.00001f){
                        hit += 1;
                        rec = tmp;
                        rec.mat = world.materials[hittables.data[i].matIndex];
                    }
                }
                break;
            case QUAD:
                struct Quad q = *((struct Quad*)hittables.data[i].data);
                if(hitQuad(r, q, &tmp)){
                    if(rec.t > tmp.t && tmp.t > 0.00001f){
                        hit += 1;
                        rec = tmp;
                        rec.mat = world.materials[hittables.data[i].matIndex];
                    }
                }
                break;
            case TRI:
                struct Triangle tri = *((struct Triangle*)hittables.data[i].data);
                if(hitTri(r, tri, &tmp)){
                    if(rec.t > tmp.t && tmp.t > 0.00001f){
                        hit += 1;
                        rec = tmp;
                        rec.mat = world.materials[hittables.data[i].matIndex];
                    }
                }
                break;
        default:
            printf("Default case\n");
            break;
        }
    }
    rec.r = r;
    rec.mat = hit? rec.mat : world.materials[0];
    free(hittables.data);
    return rec;
}

void initWorld(struct World * w){
    vectorInit(&(w->objects));
    w->tree = malloc(sizeof(struct Bvh));
}

void addSphere(struct World* world, struct Sphere* s, int matIndex){
    vectorPush(&(world->objects), (struct Hittable){.type=SPHERE, .data=s, .matIndex=matIndex});
}

void addQuad(struct World* world, struct Quad* quad, int matIndex){
    quad->n = vec3Cross(quad->u, quad->v);
    quad->normal = vec3Unit(quad->n);
    quad->D = vec3Dot(quad->normal, quad->p);
    quad->w = vec3Scale(quad->n, 1.0f/vec3Dot(quad->n,quad->n));
    vectorPush(&(world->objects), (struct Hittable){.type=QUAD, .data=quad, .matIndex=matIndex});
}

void addTri(struct World* world, struct Triangle* tri, int matIndex){
    vectorPush(&(world->objects), (struct Hittable){.type=TRI, .data=tri, .matIndex=matIndex});
}

#endif