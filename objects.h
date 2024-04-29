#ifndef OBJECTS
#define OBJECTS
#include "vec3.h"
#include "ray.h"
#include "texture.h"

enum matType{
    LAMBERT,
    METAL,
    DIELECTRIC
};

struct materialInfo{
    enum matType type;
    struct vec3 color;
    struct Texture* texture;
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

struct Sphere{
    struct vec3 center;
    float radius;
};

int hitSphere(ray r, struct Sphere s, struct hitRecord* rec){
    struct vec3 oc = vec3Sub(s.center, r.origin);
    float a = vec3Dot(r.dir, r.dir);
    float b =  -2.0 * vec3Dot(r.dir, oc);
    float c = vec3Dot(oc, oc)-s.radius*s.radius;
    float discriminant = b*b-4*a*c;
    if(discriminant < 0.0f){
        return 0;
    }
    float t  = (-b - sqrt(discriminant) ) / (2.0*a);
    rec->t = t;
    rec->normal= vec3Scale(vec3Sub(rayAt(r, t), s.center), 1.0f/s.radius);
    rec->front_face = vec3Dot(r.dir, rec->normal) > 0.0f ? 1 : -1;
    return 1;
}
#endif