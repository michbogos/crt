#include<stdio.h>
#include<stdlib.h>
#include"matrix.h"
#include"vec3.h"

int main(){
    struct vec3 vec = (struct vec3){0, 0, 1};
    float* mat = malloc(16*sizeof(float));
    matRotation(mat, (struct vec3){3.1415926, 0, 0});
    struct vec3 res = vec3matmul(vec, mat);
    printf("%f\n%f\n%f\n", res.x, res.y, res.z);
    if(matInvert(mat)){
        res = vec3matmul(res, mat);
        printf("%f\n%f\n%f\n", res.x, res.y, res.z);
    }
    free(mat);
    return 0;
}