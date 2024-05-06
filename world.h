#ifndef WORLD_H
#define WORLD_H

#include "objects.h"
#include "bvh.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

struct World{
    int size;
    int memory_size;
    int available_size;
    struct materialInfo* materials;
    struct Hittable* objects;
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
    w->objects=malloc(1024*4);
    w->available_size = 4096/sizeof(struct Hittable);
    w->size = 0;
}

void addSphere(struct World* world, struct Sphere* s, int matIndex){
    if(world->size == world->available_size){
        struct Hittable* tmp = malloc(world->available_size*sizeof(struct Hittable)*2);
        memcpy(tmp, world->objects, world->size*sizeof(struct Hittable));
        free(world->objects);
        world->objects = tmp;
        world->available_size *= 2;
    }
    world->objects[world->size] = (struct Hittable){.type=SPHERE, .data=s, .matIndex=matIndex};
    world->size+=1;
}

#endif