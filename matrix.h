#ifndef MATRIX
#define MATRIX
#include"vec3.h"
#include <stdlib.h>
#include <math.h>

void matTranslation(float* res, struct vec3 t){
    res[0] = 1;
    res[5] = 1;
    res[10] = 1;
    res[15] = 1;
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

    for(int i = 0; i < 16; i++){
        res[i] = fabsf(res[i]) > 2.0e-7 ? res[i] : 0.0f;
    }
}

void matScale(float* res, struct vec3 t){
    res[0] = t.x;
    res[5] = t.y;
    res[10] = t.z;
    res[15] = 1;
}

void matmul4x4(float* res, float* a, float* b){
    float tmp[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0};
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            for(int k = 0; k < 4; k++){
                tmp[i*4+j] += a[i*4+k]*b[k*4+j];
            }
        }
    }
    for(int i = 0; i < 16; i++){
        res[i] = tmp[i];
    }
}

int matInvert(float* m)
{
    double inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return -1;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        m[i] = inv[i] * det;

    return 1;
}

struct vec3 vec3matmul(struct vec3 a, float* mat){
    struct vec3 res;
    res.x = a.x*mat[0]+a.y*mat[1]+a.z*mat[2]+mat[3];
    res.y = a.x*mat[4]+a.y*mat[5]+a.z*mat[6]+mat[7];
    res.z = a.x*mat[8]+a.y*mat[9]+a.z*mat[10]+mat[11];
    return res;
}

#endif