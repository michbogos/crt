#ifndef MATERIAL
#define MATERIAL
#include<math.h>
#include "vec3.h"
#include "pcg_basic.h"
#include "ray.h"
#include "objects.h"
#include "world.h"

struct vec3 scatter(struct hitRecord rec, struct World world, pcg32_random_t* rng, int depth){
    ray new_ray;
    struct vec3 dir;
    struct materialInfo info = rec.mat;

    if(depth > info.max_bounces){
        return (struct vec3){0, 0, 0};
    }
    if(rec.t > 999999.9f){
        struct vec3 u_dir = vec3Unit(rec.r.dir);
        float u = 0.5f+atan2f(u_dir.z, u_dir.x)/2/3.1415926;
        float v = 0.5f+asinf(u_dir.y)/3.1415926;

        // unsigned int bytePerPixel = 3;
        // float* pixelOffset = env + (((int)(u*w) + w * (int)(v*h)) * bytePerPixel);
        // float r = pixelOffset[0];
        // float g = pixelOffset[1];
        // float b = pixelOffset[2];

        return sampleTexture(world.envMap, (struct vec3){u, v, 0.0f});
    }

    struct vec3 normal = rec.normal;

    if(info.normal != NULL){
        struct vec3 z = normal;
        struct vec3 y = vec3Unit(vec3Cross(normal, vec3Add(normal, vec3Scale(vec3RandHemisphere(normal, rng), 0.01f))));
        struct vec3 x = vec3Unit(vec3Cross(z, y));
        struct vec3 texNormal = sampleTexture(info.normal,  rec.uv);
        float nx = vec3Dot(texNormal, x);
        float ny = vec3Dot(texNormal, y);
        float nz = vec3Dot(texNormal, z);
        normal.x = nx;
        normal.y = ny;
        normal.z = nz;
    }

    switch (info.type)
    {
        case LAMBERT:
            dir = vec3Add(normal, vec3RandHemisphere(normal, rng));
            new_ray = (ray){rayAt(rec.r, rec.t), dir};
            break;
        
        case METAL:
            dir = vec3Add(vec3Unit(vec3Reflect(rec.r.dir, normal)), vec3Scale(vec3RandUnit(rng), info.fuzz));
            new_ray = (ray){rayAt(rec.r, rec.t), dir};
            break;
        case DIELECTRIC:
            float ior = rec.front_face ? 1.0f/info.ior : info.ior;
            struct vec3 udir = vec3Unit(rec.r.dir);

            float cos_theta = fminf(vec3Dot(normal, vec3Scale(udir, -1)), 1.0f);
            float sin_theta = sqrtf(1.0f-cos_theta*cos_theta);

            float r0 = (1 - info.ior) / (1 + info.ior);
            r0 = r0*r0;
            float reflectance = r0 + (1-r0)*pow((1 - cos_theta),5);

            struct vec3 refracted = ((ior*sin_theta > 1.0f) || (reflectance > unitRandf(rng))) ? vec3Reflect(udir, normal) : vec3Refract(udir, normal, ior);
            new_ray = (ray){rayAt(rec.r, rec.t), refracted};
            break;
        
        default:
            dir = vec3Add(normal, vec3RandHemisphere(normal, rng));
            new_ray = (ray){rayAt(rec.r, rec.t), dir};
            break;
    }

    //Get a hit record
    struct hitRecord hit = getHit(new_ray, world);
    struct vec3 color = scatter(hit, (struct World)world, rng, depth+1);

    struct vec3 texColor = sampleTexture(info.texture, rec.uv);
    color.x *= texColor.x;
    color.y *= texColor.y;
    color.z *= texColor.z;

    color = vec3Add(color, info.emissiveColor);
    return color;
}


#endif