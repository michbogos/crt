#ifndef UTIL
#define UTIL
#define RAND_MAX 2147483647

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#include<stdio.h>
#include<stdlib.h>
#include"vec3.h"
#include"pcg_basic.h"
#include<math.h>
#include<assert.h>

// inline void printVec(vec3 a){
//     printf("X: %f Y: %f Z: %f\n", a.x, a.y, a.z);
// }

void writeColor(float r, float g, float b){
    float rg = sqrtf(r);
    float gg = sqrtf(g);
    float bg = sqrtf(b);
    printf("%d %d %d\n", (int)(rg*255.0f), (int)(gg*255.0f), (int)(bg*255.0f));
}

void writePixel(float r, float g, float b, int x, int y, unsigned char* img, int w, int h, int ch){
    unsigned char rg = (unsigned char)((sqrtf(r))*255.0f);
    unsigned char gg = (unsigned char)((sqrtf(g))*255.0f);
    unsigned char bg = (unsigned char)((sqrtf(b))*255.0f);
    unsigned char* pixelOffset = img + (x + w * y) * ch;
    pixelOffset[0]=rg;
    pixelOffset[1]=gg;
    pixelOffset[2]=bg;
}

void writePixelf(float r, float g, float b, int x, int y, float* img, int w, int h, int ch){
    float* pixelOffset = img + (x + w * y) * ch;
    pixelOffset[0]=r;
    pixelOffset[1]=g;
    pixelOffset[2]=b;
}

float unitRandf(pcg32_random_t* rng){ //0-1 range
    return ((float)pcg32_random_r(rng))/((float)RAND_MAX);
}

float intervalRandf(float a, float b, pcg32_random_t* rng){
    return a+unitRandf(rng)*(a-b);
}

int intervalOverlap(float x0, float x1, float y0, float y1){
    if(x0 > x1){
        float tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    if(y0 > y1){
        float tmp = y0;
        y0 = y1;
        y1 = tmp;
    }
    return x0 <= y1 && y0 <= x1;
}

void get_file_data(void* ctx, const char* filename, const int is_mtl,
                          const char* obj_filename, char** data, size_t* len) {
  (void)ctx;
  if (!filename) {
    fprintf(stderr, "null filename\n");
    (*data) = NULL;
    (*len) = 0;
    return;
  }

    size_t data_len = 0;

    FILE* file;

    file = fopen(obj_filename, "r");

    assert(file != NULL);

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);  /* same as rewind(f); */

    *data = malloc(fsize + 1);
    fread(*data, fsize, 1, file);
    fclose(file);
    (*len) = fsize;
}

#endif