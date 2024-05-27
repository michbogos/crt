#ifndef OBJECTS
#define OBJECTS
#include "vec3.h"
#include "matrix.h"
#include "ray.h"
#include "texture.h"
#include <assert.h>

enum matType{
    LAMBERT,
    METAL,
    DIELECTRIC,
    EMISSIVE
};

struct __attribute__((packed))  materialInfo{
    enum matType type;
    struct vec3 color;
    struct vec3 emissiveColor;
    struct Texture* texture;
    struct Texture* normal;
    float fuzz;
    float ior;
    int max_bounces;
};

struct __attribute__((packed))  hitRecord{
    ray r;
    float t;
    int id;
    struct vec3 normal;
    struct vec3 uv;
    struct materialInfo mat;
    int front_face;
};

enum ObjectType{
    SPHERE, AABB, QUAD, TRI
};

struct __attribute__((packed)) Hittable{
    enum ObjectType type;
    int matIndex;
    int id;
    void* data;
    float* transform_matrix;
    float* inverse_matrix;
};

struct __attribute__((packed))  Sphere{
    struct vec3 center;
    float radius;
};

struct __attribute__((packed))  AABB{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;
    struct Hittable* object;
};

struct __attribute__((packed))  Quad{
    struct vec3 p;
    struct vec3 u;
    struct vec3 v;
    struct vec3 w;
    struct vec3 n;
    struct vec3 normal;
    float D;
};

struct __attribute__((packed))  Triangle{
    struct vec3 a;
    struct vec3 b;
    struct vec3 c;
    struct vec3 norma;
    struct vec3 normb;
    struct vec3 normc;
    struct vec3 uva;
    struct vec3 uvb;
    struct vec3 uvc;
};

struct __attribute__((packed))  Mesh{
    int index;
    int size;
    int matIdx;
};

// struct Mesh{
//     struct Triangle *
// }

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
    rec->front_face = vec3Dot(r.dir, rec->normal) < 0.0f ? 1 : -1;

    struct vec3 u_dir = vec3Unit(rec->normal);
    rec->uv = (struct vec3){0.5f+atan2f(u_dir.z, u_dir.x)/2/3.1415926f, 0.5f+asinf(u_dir.y)/3.1415926f, 0.0f};
    return 1;
}

int hitQuad(ray r, struct Quad quad, struct hitRecord* rec){
    float denom = vec3Dot(quad.normal, r.dir);

    if(fabs(denom)<1e-8f){
        return 0;
    }
    float t = (quad.D - vec3Dot(quad.normal, r.origin)) / denom;

    struct vec3 hitpt = vec3Sub(rayAt(r, t), quad.p);
    float alpha = vec3Dot(quad.w, vec3Cross(hitpt, quad.v));
    float beta =  vec3Dot(quad.w, vec3Cross(quad.u, hitpt));

    if(!(0.0f < alpha && alpha < 1.0f) || !(0.0f < beta && beta < 1.0f)){
        return 0;
    }

    rec->normal = quad.normal;
    rec->t = t;
    rec->front_face = vec3Dot(r.dir, rec->normal) < 0.0f ? 1 : -1;
    rec->uv = (struct vec3){alpha, beta, 0.0f};
    return 1;
}

int hitTri(ray r, struct Triangle tri, struct hitRecord* rec){
    struct vec3 v1v0 = vec3Sub(tri.b, tri.a);
    struct vec3 v2v0 = vec3Sub(tri.c, tri.a);
    struct vec3 rov0 = vec3Sub(r.origin, tri.a);
 
    struct vec3 n = vec3Cross( v1v0, v2v0);
    struct vec3 q = vec3Cross( rov0, r.dir);
    float d = 1.0/vec3Dot(  n, r.dir );
    float u =   d*vec3Dot( vec3Scale(q, -1.0f), v2v0);
    float v =   d*vec3Dot(  q, v1v0 );
    float t =   d*vec3Dot( vec3Scale(n, -1.0f), rov0);
    float w = 1.0f-u-v;

    rec->t = t;

    if(t<0.00000001){
        return 0;
    }

    rec->r = r;

    rec->normal = vec3Unit(vec3Add(vec3Scale(tri.norma, w), vec3Add(vec3Scale(tri.normb, u), vec3Scale(tri.normc, v))));

    rec->uv = vec3Add(vec3Scale(tri.uva, w), vec3Add(vec3Scale(tri.uvb, u), vec3Scale(tri.uvc, v)));
    rec->front_face = vec3Dot(r.dir, rec->normal) > 0.0f ? 1 : -1;
 
    return (u<0.0 || v<0.0 || (u+v)>1.0) ? 0 : 1;
}

struct AABB HittableAABB(struct Hittable* object){
    struct AABB res;
    switch (object->type){
        case SPHERE:
            struct Sphere s = *((struct Sphere*)(object->data));
            s.center = object->transform_matrix != NULL ? vec3matmul(s.center, object->transform_matrix) : s.center;
            res.x0 = s.center.x-s.radius;
            res.x1 = s.center.x+s.radius;
            res.y0 = s.center.y-s.radius;
            res.y1 = s.center.y+s.radius;
            res.z0 = s.center.z-s.radius;
            res.z1 = s.center.z+s.radius;
            res.object = object;
            break;
        
        case QUAD:
            struct Quad q = *((struct Quad*)(object->data));
            q.p = object->transform_matrix != NULL ? vec3matmul(q.p, object->transform_matrix) : q.p;
            struct vec3 a = vec3Add(q.p, q.u);
            struct vec3 b = vec3Add(q.p, q.v);
            struct vec3 c = vec3Add(vec3Add(q.p, q.u), q.v);
            res.x0 = MIN(q.p.x, MIN(a.x, MIN(b.x, c.x)))-0.000001;
            res.x1 = MAX(q.p.x, MAX(a.x, MAX(b.x, c.x)))+0.000001;
            res.y0 = MIN(q.p.y, MIN(a.y, MIN(b.y, c.y)))-0.000001;
            res.y1 = MAX(q.p.y, MAX(a.y, MAX(b.y, c.y)))+0.000001;
            res.z0 = MIN(q.p.z, MIN(a.z, MIN(b.z, c.z)))-0.000001;
            res.z1 = MAX(q.p.z, MAX(a.z, MAX(b.z, c.z)))+0.000001;
            res.object = object;
            break;
        
        case TRI:
            struct Triangle tri = *((struct Triangle*)(object->data));
            tri.a = object->transform_matrix != NULL ? vec3matmul(tri.a, object->transform_matrix) : tri.a;
            tri.b = object->transform_matrix != NULL ? vec3matmul(tri.b, object->transform_matrix) : tri.b;
            tri.c = object->transform_matrix != NULL ? vec3matmul(tri.c, object->transform_matrix) : tri.c;
            res.x0 = MIN(tri.a.x, MIN(tri.b.x,  tri.c.x))-0.000001;
            res.x1 = MAX(tri.a.x, MAX(tri.b.x,  tri.c.x))+0.000001;
            res.y0 = MIN(tri.a.y, MIN(tri.b.y,  tri.c.y))-0.000001;
            res.y1 = MAX(tri.a.y, MAX(tri.b.y,  tri.c.y))+0.000001;
            res.z0 = MIN(tri.a.z, MIN(tri.b.z,  tri.c.z))-0.000001;
            res.z1 = MAX(tri.a.z, MAX(tri.b.z,  tri.c.z))+0.000001;
            res.object = object;
            break;
        
        default:
            printf("Default AABB\n");
            break;
    }
    return res;
}

float HittableArea(struct Hittable* object, int axis){
    float res = 0;
    switch (object->type){
        case SPHERE:
            struct Sphere s = *((struct Sphere*)(object->data));
            res = 3.1415926*s.radius*s.radius;
            break;
        
        case QUAD:
            struct Quad q = *((struct Quad*)(object->data));
            res = vec3Mag(q.u)*vec3Mag(q.v);
            break;
        
        case TRI:
            struct Triangle tri = *((struct Triangle*)(object->data));
            tri.a = object->transform_matrix != NULL ? vec3matmul(tri.a, object->transform_matrix) : tri.a;
            tri.b = object->transform_matrix != NULL ? vec3matmul(tri.b, object->transform_matrix) : tri.b;
            tri.c = object->transform_matrix != NULL ? vec3matmul(tri.c, object->transform_matrix) : tri.c;
            res = vec3Mag(vec3Cross(vec3Sub(tri.a, tri.b), vec3Sub(tri.c, tri.b)))*0.5f;
            break;
        
        default:
            printf("Default AABB area\n");
            break;
    }
    return res;
}
#endif