#ifndef OBJECTS
#define OBJECTS

#include "vec3.h"
#include "ray.h"

struct hitRecord{
    float t;
    vec3 normal;
};

typedef struct Sphere{
    vec3 center;
    float radius;
};

int hitSphere(ray r, point3 center, float radius, struct hitRecord* rec){
    vec3 oc = vec3Sub(center, r.origin);
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
    return 1;
}

#endif