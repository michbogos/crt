#ifndef WORLD_H
#define WORLD_H

struct World{
    struct vec3* pos;
    float* radii;
    struct materialInfo* materials;
};

#include "objects.h"

//Maybe add sky as a seperate object and material
struct hitRecord getHit(ray r, struct World world){
    int hit = 0;
    struct hitRecord rec;
    rec.t = 1000000.0f;
    for(int i = 0; i < 5; i++){
        struct hitRecord tmp;
        if(hitSphere(r, world.pos[i], world.radii[i], &tmp)){
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

#endif