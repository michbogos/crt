#ifndef MATERIAL
#define MATERIAL
#include<math.h>
#include "vec3.h"
#include "pcg_basic.h"
#include "ray.h"
#include "objects.h"
#include "world.h"

struct vec3 evaluateBSDF(struct BSDFInfo bsdf, struct vec3 in, struct vec3 out, struct vec3 norm){
    struct vec3 h = vec3Scale(vec3Add(in, out), 0.5);
    float fd90 = 0.5 + 2 * bsdf.roughness * powf(vec3Dot(out, h), 2.0f);
    float fdin = (1 + (fd90 - 1) * powf((1 - fabsf(vec3Dot(norm, in))), 5.0f));
    float fdout = (1 + (fd90 - 1) * powf((1 - fabsf(vec3Dot(norm, out))), 5.0f));
    return vec3Scale(bsdf.baseColor, (1/3.1415926)*fdin*fdout*fabsf(vec3Dot(norm, out)));
}

struct vec3 linearScatter(struct hitRecord rec, struct World world, pcg32_random_t* rng, int depth){
    ray new_ray;
    struct materialInfo info;
    struct vec3 color = (struct vec3){1, 1, 1};
    struct hitRecord hit = rec;
    for(int i = 0; i <= depth; i++){
        info = hit.mat;
        if(i == depth){
            color = (struct vec3){0, 0, 0};
            break;
        }
        if(hit.t > 999999.9f){
            struct vec3 u_dir = vec3Unit(hit.r.dir);
            float u = 0.5f+atan2f(u_dir.z, u_dir.x)/2/3.1415926;
            float v = 0.5f+asinf(u_dir.y)/3.1415926;

            // unsigned int bytePerPixel = 3;
            // float* pixelOffset = env + (((int)(u*w) + w * (int)(v*h)) * bytePerPixel);
            // float r = pixelOffset[0];
            // float g = pixelOffset[1];
            // float b = pixelOffset[2];

            struct vec3 c = sampleTexture(world.textures+world.envmap, world.texture_data, (struct vec3){u, v, 0.0f});
            color.x *= c.x;
            color.y *= c.y;
            color.z *= c.z;
            break;
        }

        struct vec3 texColor = sampleTexture(world.textures+info.texture, world.texture_data, hit.uv);
        // color.x *= texColor.x;
        // color.y *= texColor.y;
        // color.z *= texColor.z;

        // color = vec3Add(color, info.emissiveColor);

        struct vec3 normal = hit.normal;

        if(info.normal > -1){
            struct vec3 z = normal;
            struct vec3 y = vec3Unit(vec3Cross(normal, vec3Add(normal, vec3Scale(vec3RandHemisphere(normal, rng), 0.01f))));
            struct vec3 x = vec3Unit(vec3Cross(z, y));
            struct vec3 texNormal = sampleTexture(world.textures+info.normal, world.texture_data,  hit.uv);
            float nx = vec3Dot(texNormal, x);
            float ny = vec3Dot(texNormal, y);
            float nz = vec3Dot(texNormal, z);
            normal.x = nx;
            normal.y = ny;
            normal.z = nz;
        }

        color.x += info.emissiveColor.x;
        color.y += info.emissiveColor.x;
        color.z += info.emissiveColor.x;

        struct BSDFInfo bsdfinfo = {.baseColor = texColor, .roughness=0.01};

        if(unitRandf(rng)<bsdfinfo.roughness){
            new_ray = (ray){rayAt(hit.r, hit.t), vec3RandHemisphere(normal, rng)};
            struct vec3 bsdfcolor = evaluateBSDF(bsdfinfo, vec3Unit(hit.r.dir), vec3Unit(new_ray.dir), normal);
            color.x *= (bsdfcolor.x)*2*3.1415926*bsdfinfo.roughness;
            color.y *= (bsdfcolor.y)*2*3.1415926*bsdfinfo.roughness;
            color.z *= (bsdfcolor.z)*2*3.1415926*bsdfinfo.roughness;
        }
        else{
            new_ray = (ray){rayAt(hit.r, hit.t), vec3Reflect(hit.r.dir, normal)};
            struct vec3 bsdfcolor = evaluateBSDF(bsdfinfo, vec3Unit(hit.r.dir), vec3Unit(new_ray.dir), normal);
            color.x *= (bsdfcolor.x)*(1-bsdfinfo.roughness);
            color.y *= (bsdfcolor.y)*(1-bsdfinfo.roughness);
            color.z *= (bsdfcolor.z)*(1-bsdfinfo.roughness);
        }

        // switch (info.type){
        //     case LAMBERT:
        //         new_ray = (ray){rayAt(hit.r, hit.t), vec3Add(normal, vec3RandHemisphere(normal, rng))};
        //         break;
            
        //     case METAL:
        //         new_ray = (ray){rayAt(hit.r, hit.t), vec3Add(vec3Unit(vec3Reflect(hit.r.dir, normal)), vec3Scale(vec3RandUnit(rng), info.fuzz))};
        //         break;
        //     case DIELECTRIC:
        //         float ior = hit.front_face ? 1.0f/info.ior : info.ior;
        //         struct vec3 udir = vec3Unit(hit.r.dir);

        //         float cos_theta = fminf(vec3Dot(normal, vec3Scale(udir, -1)), 1.0f);
        //         float sin_theta = sqrtf(1.0f-cos_theta*cos_theta);

        //         float r0 = (1 - ior) / (1 + ior);
        //         r0 = r0*r0;
        //         float reflectance = r0 + (1-r0)*pow((1 - cos_theta),5);

        //         struct vec3 refracted = ((ior*sin_theta > 1.0f) || (reflectance > unitRandf(rng))) ? vec3Reflect(udir, normal) : vec3Refract(udir, normal, ior);
        //         new_ray = (ray){rayAt(hit.r, hit.t), refracted};
        //         break;
            
        //     default:
        //         new_ray = (ray){rayAt(hit.r, hit.t), vec3Add(normal, vec3RandHemisphere(normal, rng))};
        //         break;
        // }
        hit = getHit(new_ray, world);
    }
    return color;
}

#endif