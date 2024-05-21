#ifndef BVH
#define BVH
#include"vec3.h"
#include"ray.h"
#include"objects.h"
#include"world.h"
#include"vector.h"
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
    struct AABB box;
    struct Hittable* objects;
    struct Bvh* left;
    struct Bvh* right;
    int num_objects;
    char hasChildren;
    char splitAxis; // 0 -> x; 1 -> y; 2 -> z;
};

struct LBvh{
    struct AABB* box;
    int objects;
    int left;
    int right;
};

void buildBvh(struct Bvh* bvh, struct Hittable** objects, int num_objects){
    struct AABB* boxes = calloc(num_objects, sizeof(struct AABB));
    float* areas = calloc(num_objects+1, sizeof(float));
    areas[0] = 0;
    struct AABB parent = {10e10f, -10e10f, 10e10f, -10e10f, 10e10f, -10e10f};
    bvh->hasChildren = 0;
    //Gets bounds of parent
    for(int i = 0; i < num_objects; i++){
        boxes[i] = HittableAABB(*(objects+i));
        areas[i+1] = HittableArea(*(objects+i))+areas[i];
        parent.x0 = boxes[i].x0 < parent.x0 ? boxes[i].x0 : parent.x0;
        parent.x1 = boxes[i].x1 > parent.x1 ? boxes[i].x1 : parent.x1;
        parent.y0 = boxes[i].y0 < parent.y0 ? boxes[i].y0 : parent.y0;
        parent.y1 = boxes[i].y1 > parent.y1 ? boxes[i].y1 : parent.y1;
        parent.z0 = boxes[i].z0 < parent.z0 ? boxes[i].z0 : parent.z0;
        parent.z1 = boxes[i].z1 > parent.z1 ? boxes[i].z1 : parent.z1;
    }
    float extent = MAX(parent.x1-parent.x0, MAX(parent.y1-parent.y0, parent.z1-parent.z0));
    bvh->box = parent;

    if(num_objects == 1){
        bvh->splitAxis = 0;
        bvh->hasChildren = 1;
        bvh->objects = *objects;
        bvh->num_objects = num_objects;
        return;
    }

    if(fabsf(extent-(parent.x1-parent.x0))<0.0001){
        qsort(boxes, num_objects, sizeof(struct AABB), cmpx);
        bvh->splitAxis = 0;
    }
    else if(fabsf(extent-(parent.y1-parent.y0))<0.0001){
        qsort(boxes, num_objects, sizeof(struct AABB), cmpy);
        bvh->splitAxis = 1;
    }
    else{
        qsort(boxes, num_objects, sizeof(struct AABB), cmpz);
        bvh->splitAxis = 2;
    }

    int mid = 0;
    int l = 0;
    int r = num_objects;
    float costl = 0;
    float costr = 0;
    while(r-l > 1){
        mid = (l+r)/2;
        if(areas[mid] < areas[num_objects]-areas[mid+1]){
            l = mid;
        }
        else{
            r = mid;
        }
        if(areas[mid] == areas[num_objects]-areas[mid+1]){
            break;
        }
    }

    bvh->left = (struct Bvh*)(malloc(sizeof(struct Bvh)));
    buildBvh(bvh->left, objects, mid);
    bvh->right = (struct Bvh*)(malloc(sizeof(struct Bvh)));
    buildBvh(bvh->right, objects+(mid), (num_objects-mid));

    free(boxes);
    free(areas);
}

void buildLBvh();

void traverseBvh(struct Vector* vec, struct Bvh* bvh, ray r){
    if(!intersectAABB(r, &(bvh->box))){
        return;
    }
    if(bvh->hasChildren){
        vectorPush(vec, *(bvh->objects));
        return;
    }
    switch(bvh->splitAxis){
        case 0:
            if(r.dir.x < 0){
                if(intersectAABB(r, &(bvh->left->box))){
                    traverseBvh(vec, bvh->left, r);
                }
                if(intersectAABB(r, &(bvh->right->box))){
                    traverseBvh(vec, bvh->right, r);
                }
            }
            else{
                if(intersectAABB(r, &(bvh->right->box))){
                    traverseBvh(vec, bvh->right, r);
                }
                if(intersectAABB(r, &(bvh->left->box))){
                    traverseBvh(vec, bvh->left, r);
                }
            }
            break;
        case 1:
            if(r.dir.y < 0){
                if(intersectAABB(r, &(bvh->left->box))){
                    traverseBvh(vec, bvh->left, r);
                }
                if(intersectAABB(r, &(bvh->right->box))){
                    traverseBvh(vec, bvh->right, r);
                }
            }
            else{
                if(intersectAABB(r, &(bvh->right->box))){
                    traverseBvh(vec, bvh->right, r);
                }
                if(intersectAABB(r, &(bvh->left->box))){
                    traverseBvh(vec, bvh->left, r);
                }
            }
            break;
        case 2:
            if(r.dir.z < 0){
                if(intersectAABB(r, &(bvh->left->box))){
                    traverseBvh(vec, bvh->left, r);
                }
                if(intersectAABB(r, &(bvh->right->box))){
                    traverseBvh(vec, bvh->right, r);
                }
            }
            else{
                if(intersectAABB(r, &(bvh->right->box))){
                    traverseBvh(vec, bvh->right, r);
                }
                if(intersectAABB(r, &(bvh->left->box))){
                    traverseBvh(vec, bvh->left, r);
                }
            }
            break;
        default:
        printf("default splitaxis\n");
            if(intersectAABB(r, &(bvh->left->box))){
                traverseBvh(vec, bvh->left, r);
            }
            if(intersectAABB(r, &(bvh->right->box))){
                traverseBvh(vec, bvh->right, r);
            }
    }
    return;
}

#endif