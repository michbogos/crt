#ifndef ARENA
#define ARENA
#include<string.h>
#include<stdlib.h>

struct Arena{
    char* beginning;
    char* end;
    int size;
};


void *alloc(struct Arena *a, int size)
{
    // if(a->end-a->beginning){
        
    // }
    void *p = a->beginning;
    a->beginning += size;
    return memset(p, 0, size);
}

void initArena(struct Arena* a, int size){
    a->beginning = (char*)malloc(size);
    a->end = a->beginning+size;
    a->size = size;
}

#endif