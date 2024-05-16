#ifndef CAMERA
#define CAMERA
#include "vec3.h"
#include "ray.h"
#include "pcg_basic.h"

struct Camera{
    struct vec3 pos;
    struct vec3 look_at;
    struct vec3 camera_up;
    float fov;
    float focus;
    float defocus_angle;
    float focal_length;

    float aspect_ratio;
    float h;
    float viewport_height;
    float viewport_width;
    float defocus_radius;
    struct vec3 u;
    struct vec3 v;
    struct vec3 w;

    struct vec3 viewport_u;
    struct vec3 viewport_v;
    struct vec3 du;
    struct vec3 dv;
    struct vec3 defocus_disk_u;
    struct vec3 defocus_disk_v;
    struct vec3 top_left;
};

void initCamera(struct Camera* cam, int width, int height){
    cam->aspect_ratio = (float)width/(float)height;
    cam->h = tanf(cam->fov/2);

    cam->focal_length = vec3Mag(vec3Sub(cam->pos, cam->look_at));
    cam->focus = 0.01f;
    cam->defocus_angle = -1;
    cam->defocus_radius = cam->focus*tanf(cam->defocus_angle/2);

    cam->viewport_height = 2*cam->h*cam->focal_length;
    cam->viewport_width = cam->viewport_height*cam->aspect_ratio;
    cam->w = vec3Unit(vec3Sub(cam->pos, cam->look_at));
    cam->u = vec3Unit(vec3Cross(cam->camera_up, cam->w));
    cam->v = vec3Cross(cam->w, cam->u);

    cam->viewport_u = vec3Scale(cam->u, cam->viewport_width);
    cam->viewport_v = vec3Scale(cam->v, cam->viewport_height);
    cam->du = vec3Scale(cam->viewport_u, 1.0f/width);
    cam->dv = vec3Scale(cam->viewport_v, 1.0f/height);
    cam->defocus_disk_u = vec3Scale(cam->u, cam->defocus_radius);
    cam->defocus_disk_v = vec3Scale(cam->v, cam->defocus_radius);
    cam->top_left = vec3Sub(vec3Sub(vec3Sub(cam->pos, vec3Scale(cam->w, -cam->focal_length)), vec3Scale(cam->viewport_u, 0.5)), vec3Scale(cam->viewport_v, 0.5));
}

ray getRay(struct Camera cam, int i, int j, pcg32_random_t* rng){
    struct vec3 dest = vec3Add(cam.pos, vec3Add(vec3Add(cam.top_left, vec3Scale(cam.du, i)), vec3Scale(cam.dv, j)));
    // float y = ((float)j/(float)(HEIGHT))*h*2;
    // float z = 0;
    ray r;
    struct vec3 p = vec3RandDisc(rng);
    struct vec3 defocus_sample = vec3Add(cam.pos, vec3Add(vec3Scale(cam.defocus_disk_u, p.x), vec3Scale(cam.defocus_disk_v, p.y)));
    r.origin = (cam.defocus_angle <= 0) ? cam.pos : defocus_sample;
    r.dir = vec3Sub(r.origin, dest);

    return r;
}

#endif