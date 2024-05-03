#ifndef BVH
#define BVH
#include"vec3.h"
#include"ray.h"
#include"objects.h"
#include"world.h"

int cmpx (const void * a, const void * b) {
    struct AABB boxa = *((struct AABB*)a);
    struct AABB boxb = *((struct AABB*)b);
    return ((boxa.x0+boxa.x1)*0.5f>(boxb.x0+boxb.x1)*0.5);
}

int cmpy (const void * a, const void * b) {
    struct AABB boxa = *((struct AABB*)a);
    struct AABB boxb = *((struct AABB*)b);
    return ((boxa.y0+boxa.y1)*0.5f>(boxb.y0+boxb.y1)*0.5);
}

int cmpz (const void * a, const void * b) {
    struct AABB boxa = *((struct AABB*)a);
    struct AABB boxb = *((struct AABB*)b);
    return ((boxa.z0+boxa.z1)*0.5f>(boxb.z0+boxb.z1)*0.5);
}

struct Bvh{
    struct Bvh* children;
    struct AABB;
    char hasChildren;
    struct Hittable* object;
};

void buldBvh(struct Bvh* bvh, struct Hittable* objects, int num_objects){
    struct AABB boxes[num_objects];
    struct AABB parent = {10e10f, -10e10f, 10e10f, -10e10f, 10e10f, -10e10f};
    //Gets bounds of parent
    for(int i = 0; i < num_objects; i++){
        boxes[i] = HittableAABB(objects+i);
        parent.x0 = boxes[i].x0 < parent.x0 ? boxes[i].x0 : parent.x0;
        parent.x1 = boxes[i].x1 > parent.x1 ? boxes[i].x1 : parent.x1;
        parent.y0 = boxes[i].y0 < parent.y0 ? boxes[i].y0 : parent.y0;
        parent.y1 = boxes[i].y1 > parent.y1 ? boxes[i].y1 : parent.y1;
        parent.z0 = boxes[i].z0 < parent.z0 ? boxes[i].z0 : parent.z0;
        parent.z1 = boxes[i].z1 > parent.z1 ? boxes[i].z1 : parent.z1;
    }

    // qsort(boxes, num_objects, sizeof(struct AABB), cmpx);
}

#endif