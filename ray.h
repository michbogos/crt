
#ifndef RAY
#define RAY
#include "vec3.h"

typedef struct {
    struct vec3 origin;
    struct vec3 dir;
} ray;

struct vec3 rayAt(ray r, float t){
    return vec3Add(r.origin, vec3Scale(r.dir, t));
}

#endif