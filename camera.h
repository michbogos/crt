#ifndef CAMERA
#define CAMERA
#include "vec3.h"

struct Camera{
    struct vec3 pos;
    struct vec3 lookAt;
    float fov;
    float fstop;
};

#endif