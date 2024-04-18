#ifndef UTIL
#define UTIL

#include<stdio.h>
#include"vec3.h"
#include"pcg_basic.h"
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

float unitRandf(pcg32_random_t* rng){ //0-1 range
    return ((float)pcg32_random_r(rng))/((float)(1L<<32)-1);
}

float intervalRandf(float a, float b, pcg32_random_t* rng){
    return a+unitRandf(rng)*(a-b);
}

#endif