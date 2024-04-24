#ifndef VECTOR
#define VECTOR

#include<stdlib.h>

struct Vector{
    void** data;
    int size;
    int element_size;
    int num_elements;
};

void vectorInit(struct Vector* vec, int element_size){
    vec->data = (void**)malloc(1024);
    vec->element_size = element_size;
}

void vectorPush(struct Vector* vec, void* a){
    if(vec->element_size*(vec->num_elements+1) < vec->size){
        for(int i = 0; i < vec->element_size; i++){
            vec->data[vec->num_elements*vec->element_size+i] = a+i;
        }
    }
}

#endif