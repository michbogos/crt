#ifndef VECTOR
#define VECTOR

#include<stdlib.h>
#include<string.h>
#include"objects.h"

struct Vector{
    struct Hittable* data;
    int size;
    int available_size;
};

void vectorInit(struct Vector* vec){
    vec->data = (struct Hittable*)malloc(1024*16);
    vec->available_size = 1024*16;
    vec->size = 0;
}

void vectorPush(struct Vector* vec, struct Hittable data){
    if(vec->available_size < vec->size*sizeof(struct Hittable)){
        struct Hittable* tmp = malloc(vec->available_size*2);
        memcpy(vec->data, tmp, sizeof(struct Hittable)*vec->size);
        free(vec->data);
        vec->data = tmp;
        vec->available_size *=2;
    }
    vec->data[vec->size] = data;
    vec->size += 1;
}

#endif