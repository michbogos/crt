#ifndef MATERIAL
#define MATERIAL
#include "vec3.h"
#include "pcg_basic.h"
#include "ray.h"
#include "objects.h"

struct hitRecord getHit(ray r);

//Evil function pointer passing
struct vec3 scatter(struct hitRecord rec, pcg32_random_t* rng, int depth){
    ray new_ray;
    struct vec3 dir;
    struct materialInfo info = rec.mat;

    if(depth > info.max_bounces){
        return (struct vec3){0, 0, 0};
    }
    if(rec.t > 99999.9f){
        struct vec3 u_dir = vec3Unit(rec.r.dir);
        float a = 0.5f * (u_dir.y+1);
        return (struct vec3){a*0.5, a*0.7, a*1.0};
    }

    switch (info.type)
    {
        case LAMBERT:
            dir = vec3Add(rec.normal, vec3RandHemisphere(rec.normal, rng));
            new_ray = (ray){rayAt(rec.r, rec.t), dir};
            break;
        
        case METAL:
            dir = vec3Add(vec3Unit(vec3Reflect(rec.r.dir, rec.normal)), vec3Scale(vec3RandUnit(rng), info.fuzz));
            new_ray = (ray){rayAt(rec.r, rec.t), dir};
            break;
        case DIELECTRIC:
            float ior = rec.front_face ? 1.0f/info.ior : info.ior;
            struct vec3 udir = vec3Unit(rec.r.dir);

            float cos_theta = fminf(vec3Dot(rec.normal, vec3Scale(udir, -1)), 1.0f);
            float sin_theta = sqrtf(1.0f-cos_theta*cos_theta);

            struct vec3 refracted = ior*sin_theta < 1.0f ? vec3Refract(udir, rec.normal, ior) : vec3Reflect(udir, rec.normal);
            new_ray = (ray){rayAt(rec.r, rec.t), refracted};
            break;
        
        default:
            dir = vec3Add(rec.normal, vec3RandHemisphere(rec.normal, rng));
            new_ray = (ray){rayAt(rec.r, rec.t), dir};
            break;
    }

    //Get a hit record
    struct hitRecord hit = getHit(new_ray);
    struct vec3 color = scatter(hit, rng, depth+1);
    color.x *= info.color.x;
    color.y *= info.color.y;
    color.z *= info.color.z;
    return color;
}


#endif