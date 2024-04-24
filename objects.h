#ifndef OBJECTS
#define OBJECTS
#include "vec3.h"
#include "ray.h"

enum matType{
    LAMBERT,
    METAL,
    DIELECTRIC
};

struct materialInfo{
    enum matType type;
    struct vec3 color;
    float fuzz;
    float ior;
    int max_bounces;
};

struct hitRecord{
    ray r;
    float t;
    int id;
    struct vec3 normal;
    struct materialInfo mat;
    int front_face;
};

int hitSphere(ray r, struct vec3 center, float radius, struct hitRecord* rec){
    struct vec3 oc = vec3Sub(center, r.origin);
    float a = vec3Dot(r.dir, r.dir);
    float b =  -2.0 * vec3Dot(r.dir, oc);
    float c = vec3Dot(oc, oc)-radius*radius;
    float discriminant = b*b-4*a*c;
    if(discriminant < 0.0f){
        return 0;
    }
    float t  = (-b - sqrt(discriminant) ) / (2.0*a);
    rec->t = t;
    rec->normal= vec3Scale(vec3Sub(rayAt(r, t), center), 1.0f/radius);
    rec->front_face = vec3Dot(r.dir, rec->normal) > 0.0f ? 1 : -1;
    return 1;
}
#endif