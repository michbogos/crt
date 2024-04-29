#ifndef WORLD_H
#define WORLD_H

#include "objects.h"

struct World{
    struct materialInfo* materials;
    struct Sphere* spheres;
};

//Maybe add sky as a seperate object and material
struct hitRecord getHit(ray r, struct World world){
    int hit = 0;
    struct hitRecord rec;
    rec.t = 1000000.0f;
    for(int i = 0; i < 5; i++){
        struct hitRecord tmp;
        if(hitSphere(r, world.spheres[i], &tmp)){
            if(rec.t > tmp.t && tmp.t > 0.00001f){
                hit += 1;
                rec = tmp;
                rec.id = i;
            }
        }
    }
    rec.r = r;
    rec.mat = hit ? world.materials[rec.id] : world.materials[0];
    return rec;
}

void addSphere(struct World* world, struct Sphere s);

#endif