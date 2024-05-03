#ifndef WORLD_H
#define WORLD_H

#include "objects.h"
#include <stdlib.h>
#include <string.h>

struct Hittable{
    enum ObjectType type;
    int matIndex;
    void* data;
};

struct World{
    int size;
    int memory_size;
    int available_size;
    struct materialInfo* materials;
    struct Hittable* objects;
};

//Maybe add sky as a seperate object and material
struct hitRecord getHit(ray r, struct World world){
    int hit = 0;
    struct hitRecord rec;
    rec.t = 1000000.0f;
    for(int i = 0; i < world.size; i++){
        struct hitRecord tmp;
        switch (world.objects[i].type)
        {
        case SPHERE:
        struct Sphere s = *((struct Sphere*)world.objects[i].data);
            if(hitSphere(r, s, &tmp)){
                if(rec.t > tmp.t && tmp.t > 0.00001f){
                    hit += 1;
                    rec = tmp;
                    rec.id = i;
                }
            }
            break;
        
        default:
            printf("Default case\n");
            break;
        }
    }
    rec.r = r;
    rec.mat = hit ? world.materials[world.objects[rec.id].matIndex] : world.materials[0];
    return rec;
}

void initWorld(struct World * w){
    w->objects=malloc(1024);
    w->available_size = 1024/sizeof(struct Hittable);
    w->size = 0;
}

void addSphere(struct World* world, struct Sphere* s, int matIndex){
    if(world->size == world->available_size){
        struct Hittable* tmp = malloc(world->available_size*sizeof(struct Hittable)*2);
        memcpy(tmp, world->objects, world->size*sizeof(struct Hittable));
        world->objects = tmp;
        world->available_size *= 2;
    }
    world->objects[world->size] = (struct Hittable){.type=SPHERE, .data=s, .matIndex=matIndex};
    world->size+=1;
}

#endif