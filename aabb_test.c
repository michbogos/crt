#include"vec3.h"
#include"ray.h"
#include"objects.h"
#include"util.h"
#include<stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(){
    struct AABB box = {.x0=-5, .x1=5, .y0=-5, .y1=5, .z0=-5, .z1=5};
    ray r = {.origin=(struct vec3){10, 10, 10}, .dir=(struct vec3){1, 0, 1}};
    printf("Colission: %d\n", intersectAABB(r, &box));
    return 0;
}