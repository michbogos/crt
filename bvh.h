#ifndef BVH
#define BVH
#include"vec3.h"
#include"ray.h"
#include"objects.h"
#include"world.h"
#include<stdlib.h>

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
    struct Bvh* left;
    struct Bvh* right;
    struct AABB box;
    char hasChildren;
    struct Hittable** objects;
    int num_objects;
};

void buildBvh(struct Bvh* bvh, struct Hittable** objects, int num_objects){
    struct AABB boxes[num_objects];
    struct AABB parent = {10e10f, -10e10f, 10e10f, -10e10f, 10e10f, -10e10f};
    bvh->hasChildren = 0;
    //Gets bounds of parent
    for(int i = 0; i < num_objects; i++){
        boxes[i] = HittableAABB(*(objects+i));
        parent.x0 = boxes[i].x0 < parent.x0 ? boxes[i].x0 : parent.x0;
        parent.x1 = boxes[i].x1 > parent.x1 ? boxes[i].x1 : parent.x1;
        parent.y0 = boxes[i].y0 < parent.y0 ? boxes[i].y0 : parent.y0;
        parent.y1 = boxes[i].y1 > parent.y1 ? boxes[i].y1 : parent.y1;
        parent.z0 = boxes[i].z0 < parent.z0 ? boxes[i].z0 : parent.z0;
        parent.z1 = boxes[i].z1 > parent.z1 ? boxes[i].z1 : parent.z1;
    }
    float extent = MAX(parent.x1-parent.x0, MAX(parent.y1-parent.y0, parent.z1-parent.z0));
    bvh->box = parent;

<<<<<<< HEAD
    if(num_objects <= 2){
        if(num_objects == 0){
            bvh->hasChildren = -1;
            return;
        }
        bvh->hasChildren = 1;
        bvh->objects = objects;
=======
    if(num_objects = 1){
        bvh->hasChildren = 1;
        bvh->objects = *objects;
>>>>>>> 3caedaf (Update repo)
        bvh->num_objects = num_objects;
        return;
    }

    if(fabsf(extent-(parent.x1-parent.x0))<0.0001){
        qsort(boxes, num_objects, sizeof(struct AABB), cmpx);
        struct Bvh* left_bvh = (struct Bvh*)(malloc(sizeof(struct Bvh)));
        buildBvh(left_bvh, objects, num_objects/2);
        struct Bvh* right_bvh = (struct Bvh*)(malloc(sizeof(struct Bvh)));
        buildBvh(right_bvh, objects+(num_objects/2), (num_objects+1)/2);
        bvh->left = left_bvh;
        bvh->right = right_bvh;
    }
    else if((fabsf(extent-(parent.y1-parent.y0))<0.0001)){
        qsort(boxes, num_objects, sizeof(struct AABB), cmpy);
        struct Bvh* left_bvh = (struct Bvh*)(malloc(sizeof(struct Bvh)));
        buildBvh(left_bvh, objects, num_objects/2);
        struct Bvh* right_bvh = (struct Bvh*)(malloc(sizeof(struct Bvh)));
        buildBvh(right_bvh, objects+(num_objects/2), (num_objects+1)/2);
        bvh->left = left_bvh;
        bvh->right = right_bvh;
    }
    else if((fabsf(extent-(parent.z1-parent.z0))<0.0001)){
        qsort(boxes, num_objects, sizeof(struct AABB), cmpz);
        struct Bvh* left_bvh = (struct Bvh*)(malloc(sizeof(struct Bvh)));
        buildBvh(left_bvh, objects, num_objects/2);
        struct Bvh* right_bvh = (struct Bvh*)(malloc(sizeof(struct Bvh)));
        buildBvh(right_bvh, objects+(num_objects/2), (num_objects+1)/2);
        bvh->left = left_bvh;
        bvh->right = right_bvh;
    }
}

#endif