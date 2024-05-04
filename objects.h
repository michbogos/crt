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

enum ObjectType{
    SPHERE, AABB
};

struct Hittable{
    enum ObjectType type;
    int matIndex;
    void* data;
};

struct Sphere{
    struct vec3 center;
    float radius;
};

struct AABB{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;
    struct Hittable* object;
};

int intersectAABB(ray r, struct AABB* aabb){
    float tx0 = (aabb->x0-r.origin.x)/r.dir.x;
    float tx1 = (aabb->x1-r.origin.x)/r.dir.x;
    float ty0 = (aabb->y0-r.origin.y)/r.dir.y;
    float ty1 = (aabb->y1-r.origin.y)/r.dir.y;
    float tz0 = (aabb->z0-r.origin.z)/r.dir.z;
    float tz1 = (aabb->z1-r.origin.z)/r.dir.z;

    return intervalOverlap(tx0, tx1, ty0, ty1) && intervalOverlap(ty0, ty1, tz0, tz1) && intervalOverlap(tx0, tx1, tz0, tz1);
}

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

struct AABB HittableAABB(struct Hittable* object){
    switch (object->type)
    {
    case SPHERE:
        struct Sphere s = *((struct Sphere*)(object->data));
        struct AABB res;
        res.x0 = s.center.x-s.radius;
        res.x1 = s.center.x+s.radius;
        res.y0 = s.center.y-s.radius;
        res.y1 = s.center.y+s.radius;
        res.z0 = s.center.z-s.radius;
        res.z1 = s.center.z+s.radius;
        res.object = object;
        return res;
        break;
    
    default:
        printf("Default AABB\n");
        break;
    }
    return (struct AABB){};
}
#endif