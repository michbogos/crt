#ifndef UTIL
#define UTIL

#include<stdio.h>
#include"vec3.h"
#include<stdlib.h>
#include<math.h>

// inline void printVec(vec3 a){
//     printf("X: %f Y: %f Z: %f\n", a.x, a.y, a.z);
// }

void writeColor(float r, float g, float b){
    float rg = sqrtf(r);
    float gg = sqrtf(g);
    float bg = sqrtf(b);
    printf("%d %d %d\n", (int)(rg*255.0f), (int)(gg*255.0f), (int)(bg*255.0f));
}

float unitRandf(){
    return rand()/(RAND_MAX+1.0f);
}

float intervalRandf(float a, float b){
    return a+unitRandf()*(a-b);
}

#endif