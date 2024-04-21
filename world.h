#ifndef WORLD_H
#define WORLD_H
#include "material.h"
#include "objects.h"

struct World{
    vec3 pos[];
    float radii[];
    struct materialInfo materials[];
};


#endif