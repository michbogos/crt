#ifndef VEC3
#define VEC3
#include<math.h>
#include"util.h"
#include"pcg_basic.h"

struct vec3{
    float x;
    float y;
    float z;
};

struct vec3 vec3Add(struct vec3 a, struct vec3 b){
    struct vec3 c;
    c.x = a.x+b.x;
    c.y = a.y+b.y;
    c.z = a.z+b.z;
    return c;
}

struct vec3 vec3Sub(struct vec3 a, struct vec3 b){
    struct vec3 c;
    c.x = a.x-b.x;
    c.y = a.y-b.y;
    c.z = a.z-b.z;
    return c;
}

struct vec3 vec3Scale(struct vec3 a, float fac){
    struct vec3 c;
    c.x = a.x*fac;
    c.y = a.y*fac;
    c.z = a.z*fac;
    return c;
}

float vec3Dot(struct vec3 a, struct vec3 b){
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

struct vec3 vec3Cross(struct vec3 a, struct vec3 b){
    struct vec3 c;
    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;
    return c;
}

  float vec3Mag(struct vec3 a){
    return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}

  float vec3Mag2(struct vec3 a){
    return a.x*a.x + a.y*a.y + a.z*a.z;
}

struct vec3 vec3Unit(struct vec3 a){
    return vec3Scale(a, 1.0f/vec3Mag(a));
}

  float vec3Dist(struct vec3 a, struct vec3 b){
    struct vec3 d = vec3Sub(a, b);
    return sqrtf(d.x*d.x + d.y*d.y+ d.z*d.z);
}

 struct vec3 vec3RandUnit(pcg32_random_t* rng){
    return vec3Unit((struct vec3){unitRandf(rng), unitRandf(rng), unitRandf(rng)});
}

struct vec3 vec3RandHemisphere(struct vec3 normal, pcg32_random_t* rng){
    while(1){
        struct vec3 v = vec3RandUnit(rng);
        if(vec3Mag(v) <= 1.0){
            if(vec3Dot(normal, v)>0){
                return v;
            }
            else{
                return vec3Scale(v, -1);
            }
        }
    }
}

struct vec3 vec3Reflect(struct vec3 a, struct vec3 normal){
    return vec3Sub(a, vec3Scale(normal, 2*vec3Dot(a,normal)));
}

struct vec3 vec3Refract(struct vec3 a, struct vec3 normal, float ior){
    float cos_theta = fminf(
            vec3Dot(vec3Scale(a, -1), normal), 1.0f
        );
    struct vec3 r_perp = vec3Scale(vec3Add(a, vec3Scale(normal, cos_theta)), ior);
    struct vec3 r_par = vec3Scale(normal, -sqrtf(fabsf(1.0-vec3Mag2(r_perp))));
    return vec3Add(r_perp, r_par);
}

#endif