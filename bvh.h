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
 
struct __attribute__((packed, aligned(4))) LBvh{
    struct Hittable* object;
    int box_idx;
    int left;
    int right;
    int axis;
    long padding;
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

    // int mid = 0;
    // int l = 0;
    // int r = num_objects;
    // float costl = 0;
    // float costr = 0;
    // while(r-l > 1){
    //     mid = (l+r)/2;
    //     if(areas[mid] < areas[num_objects]-areas[mid+1]){
    //         l = mid;
    //     }
    //     else{
    //         r = mid;
    //     }
    //     if(areas[mid] == areas[num_objects]-areas[mid+1]){
    //         break;
    //     }
    // }

    int mid = num_objects/2;

    bvh->left = (struct Bvh*)(malloc(sizeof(struct Bvh)));
    buildBvh(bvh->left, objects, mid);
    bvh->right = (struct Bvh*)(malloc(sizeof(struct Bvh)));
    buildBvh(bvh->right, objects+(mid), (num_objects-mid));

    free(boxes);
    free(areas);
}

int countNodes(struct Bvh* bvh){
    int count = 0;
    if(bvh->hasChildren){
        return 1;
    }
    count += countNodes(bvh->left)+1;
    count += countNodes(bvh->right)+1;
    return count;
}

//lbvh_array and box_array filled up;
int buildLBvh(struct LBvh lbvh_array[], struct AABB box_array[], struct Bvh* tree, int count){
    struct Bvh* bvh = tree;
    while(!tree->hasChildren){
    struct LBvh lbvh;
    if(tree->hasChildren){
        lbvh.object = tree->objects;
        lbvh.left = -1;
        lbvh.right = -1;
        lbvh.box_idx = count;
        lbvh_array[count] = lbvh;
        box_array[count] = tree->box;
        return count;
    }
    box_array[count] = tree->box;
    lbvh.box_idx = count;
    lbvh.object = NULL;
    lbvh.axis = tree->splitAxis;
    lbvh.left = count+1;
    int cnt = buildLBvh(lbvh_array, box_array, tree->left, count+1);
    lbvh.right = cnt+1;
    buildLBvh(lbvh_array, box_array, tree->right, cnt+1);
    lbvh_array[count] = lbvh;
    }
}

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

void traverseLBvh(struct Vector* vec, struct LBvh* nodes, struct AABB* boxes, ray r){
    int current_node = 0;

    int* to_visit = malloc(2048*sizeof(int));
    int to_visit_size = 1;
    int to_visit_available = 2048;
    int beginning = 0;

    to_visit[0] = 0;

    while(beginning < to_visit_size){
        for(int i = beginning; i < to_visit_size; i++){
            int node = to_visit[i];
            assert(node != -1);
            beginning += 1;
            if(nodes[node].object != NULL){
                vectorPush(vec, *(nodes[node].object));
            }
            int left = nodes[node].left;
            int right = nodes[node].right;
            if(left != -1){
                if(intersectAABB(r, boxes+nodes[left].box_idx)){
                    if(to_visit_size+1 == to_visit_available){
                        to_visit_available *= 2;
                        int* tmp = malloc(to_visit_available*sizeof(int));
                        memcpy(tmp, to_visit, to_visit_size);
                        free(to_visit);
                        to_visit = tmp;
                    }
                    to_visit[to_visit_size] = left;
                    to_visit_size++;
                }
            }
            if(right != -1){
            if(intersectAABB(r, boxes+nodes[right].box_idx)){
                if(to_visit_size+1 == to_visit_available){
                    to_visit_available *= 2;
                    int* tmp = malloc(to_visit_available*sizeof(int));
                    memcpy(tmp, to_visit, to_visit_size);
                    free(to_visit);
                    to_visit = tmp;
                }
                to_visit[to_visit_size] = right;
                to_visit_size++;
            }
        }
    }
    free(to_visit);
}
}

#endif