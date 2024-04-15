
#ifndef RAY
#define RAY

#include "vec3.h"

struct ray{
    point3 origin;
    vec3 dir;
}

typedef ray;

point3 rayAt(ray r, float t){
    return vec3Add(r.origin, vec3Scale(r.dir, t));
}

#endif