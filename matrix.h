#ifndef MATRIX
#define MATRIX
#include"vec3.h"
#include <stdlib.h>
#include <math.h>

void matTranslation(float* res, struct vec3 t){
    res[3] = t.x;
    res[7] = t.y;
    res[11] = t.z;
    res[15] = 1.0f;
}

void matRotation(float* res, struct vec3 t){
    res[0] = cosf(t.z)*cosf(t.y); 
    res[1] = cosf(t.z)*sinf(t.y)*sinf(t.x)-sinf(t.z)*cosf(t.x);
    res[2] = cosf(t.z)*sinf(t.y)*cosf(t.x)+sinf(t.z)*sinf(t.x);
    res[4] = sinf(t.z)*cosf(t.y);
    res[5] = sinf(t.z)*sinf(t.y)*sinf(t.x)+cosf(t.z)*cosf(t.x);
    res[6] = sinf(t.z)*sinf(t.y)*cosf(t.x)-cosf(t.z)*sinf(t.x);
    res[8] = -sinf(t.y);
    res[9] = cosf(t.y)*sinf(t.x);
    res[10] = cosf(t.y)*cosf(t.x);
    res[15] = 1;
}

void matScale(float* res, struct vec3 t){
    res[0] = t.x;
    res[5] = t.y;
    res[10] = t.z;
    res[15] = 1;
}

void matmul4x4(float* res, float* a, float* b){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            for(int k = 0; k < 4; k++){
                res[i*4+j] = a[i*4+k]*b[k*4+j];
            }
        }
    }
}

struct vec3 vec3matmul(struct vec3 a, float* mat){
    struct vec3 res;
    res.x = a.x*mat[0]+a.y*mat[1]+a.z*mat[2]+mat[3];
    res.y = a.x*mat[4]+a.y*mat[5]+a.z*mat[6]+mat[7];
    res.z = a.x*mat[8]+a.y*mat[9]+a.z*mat[10]+mat[11];
    return res;
}

#endif