#ifndef VECTOR
#define VECTOR

#include<math.h>

struct vec3{
    float x;
    float y;
    float z;
}

typedef vec3;
typedef vec3 point3;

inline vec3 vec3Add(vec3 a, vec3 b){
    vec3 c;
    c.x = a.x+b.x;
    c.y = a.y+b.y;
    c.z = a.z+b.z;
    return c;
}

inline vec3 vec3Sub(vec3 a, vec3 b){
    vec3 c;
    c.x = a.x-b.x;
    c.y = a.y-b.y;
    c.z = a.z-b.z;
    return c;
}

inline vec3 vec3Scale(vec3 a, float fac){
    vec3 c;
    c.x = a.x*fac;
    c.y = a.y*fac;
    c.z = a.z*fac;
    return c;
}

inline float vec3Dot(vec3 a, vec3 b){
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

inline vec3 vec3Cross(vec3 a, vec3 b){
    vec3 c;
    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;
    return c;
}

inline float vec3Mag(vec3 a){
    return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}

inline float vec3Mag2(vec3 a){
    return a.x*a.x + a.y*a.y + a.z*a.z;
}

inline vec3 vec3Unit(vec3 a){
    return vec3Scale(a, 1.0f/vec3Mag(a));
}

inline float vec3Dist(vec3 a, vec3 b){
    vec3 d = vec3Sub(a, b);
    return sqrtf(d.x*d.x + d.y*d.y+ d.z*d.z);
}

#endif