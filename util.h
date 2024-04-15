#ifndef UTIL
#define UTIL

#include<stdio.h>
#include"vec3.h"

inline void printVec(vec3 a){
    printf("X: %f Y: %f Z: %f\n", a.x, a.y, a.z);
}

inline void writeColor(unsigned char r, unsigned char g, unsigned char b){
    printf("%d %d %d\n", r, g, b);
}


#endif