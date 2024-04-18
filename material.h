#ifndef MATERIAL
#define MATERIAL

#include "objects.h"
#include "vec3.h"
#include "pcg_basic.h"
#include "ray.h"

struct materialInfo{
    vec3 color;
    float attenuation = 0.5;
    int max_bounces = 10;
};

struct vec3 scatter(struct hitRecord rec, struct materialInfo info, pcg32_random_t* rng){
    struct vec3 dir = vec3Add(rec.normal, vec3RandHemisphere(rec.normal, rng));
    ray new_ray = (ray){rayAt(rec.r, rec.t), dir};
    return vectorscatter();
}

#endif