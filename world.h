#ifndef WORLD_H
#define WORLD_H

#include "objects.h"

struct Hittable{
    enum ObjectType type;
    void* data;
};

struct World{
    int size;
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
    rec.mat = hit ? world.materials[rec.id] : world.materials[0];
    return rec;
}

void addSphere(struct World* world, struct Sphere s);

#endif