#include<stdio.h>
#include<math.h>
#include"vec3.h"
#include"ray.h" 
#include"objects.h"
#include"pcg_basic.h"


int WIDTH =  1024;
int HEIGHT =  1024;
int SAMPLES =  100;
int DEPTH =  10;


// struct materialInfo lambert = {.attenuation = 0.5, .max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL, .fuz};
// struct materialInfo metal = {.attenuation = 0.5, .max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL};

struct vec3 centers[] = {(struct vec3){0,0,-0.2}, (struct vec3){1, 1, 2}, (struct vec3){0.3, 0.3, -1}, (struct vec3){2, 2 ,0}, (struct vec3){-2, -2, 0}};
float radii[] = {0.5, 0.6, 0.3, 0.1, 2};
struct materialInfo mats[] = {(struct materialInfo){.max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT},
                              (struct materialInfo){.max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL, .fuzz=0.2f},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.9, 0.9}, .type=DIELECTRIC, .ior=1.133f},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT},
                              (struct materialInfo){.max_bounces=10, .color={0.7, 0.2, 0.7}, .type=METAL, .fuzz=0.2f}};



pcg32_random_t rng;

//Maybe add sky as a seperate object and material
struct hitRecord getHit(ray r){
    int hit = 0;
    struct hitRecord rec;
    rec.t = 1000000.0f;
    for(int i = 0; i < 5; i++){
        struct hitRecord tmp;
        if(hitSphere(r, centers[i], radii[i], &tmp)){
            if(rec.t > tmp.t && tmp.t > 0.00001f){
                hit += 1;
                rec = tmp;
                rec.id = i;
            }
        }
    }
    rec.r = r;
    rec.mat = hit ? mats[rec.id] : mats[0];
    return rec;
}

#include "material.h"
#include"util.h"

int main(){
     pcg32_srandom_r(&rng, 42u, 54u); // Constant seed


    float aspectRatio = (float)WIDTH/(float)HEIGHT;
    float fov = 1.5f;
    float h = tanf(fov/2);
    struct vec3 camera_up = (struct vec3){0, 1, 0};
    struct vec3 look_at = (struct vec3){0, 0, 0};
    struct vec3 center = (struct vec3){0, 0, -7};

    float focal_length = vec3Mag(vec3Sub(center, look_at));
    float focus = 0.01f;
    float defocus_angle = -1;
    float defocus_radius = focus*tanf(defocus_angle/2);


    float viewport_height = 2*h*focal_length;
    float viewport_width = viewport_height*aspectRatio;
    struct vec3 w = vec3Unit(vec3Sub(center, look_at));
    struct vec3 u = vec3Unit(vec3Cross(camera_up, w));
    struct vec3 v = vec3Cross(w, u);

    struct vec3 viewport_u = vec3Scale(u, viewport_width);
    struct vec3 viewport_v = vec3Scale(v, viewport_height);
    struct vec3 du = vec3Scale(viewport_u, 1.0f/WIDTH);
    struct vec3 dv = vec3Scale(viewport_v, 1.0f/HEIGHT);
    struct vec3 defocus_disk_u = vec3Scale(u, defocus_radius);
    struct vec3 defocus_disk_v = vec3Scale(v, defocus_radius);
    struct vec3 top_left = vec3Sub(vec3Sub(vec3Sub(center, vec3Scale(w, -focal_length)), vec3Scale(viewport_u, 0.5)), vec3Scale(viewport_v, 0.5));
    printf("P3\n%d %d\n255\n", WIDTH, HEIGHT);
    for(int  j = 0 ; j < HEIGHT; j++){
        fprintf(stderr, "\r%d\\%d", j, HEIGHT);
        for(int i = 0 ;i < WIDTH; i++){
            struct vec3 dest = vec3Add(center, vec3Add(vec3Add(top_left, vec3Scale(du, i)), vec3Scale(dv, j)));
            // float y = ((float)j/(float)(HEIGHT))*h*2;
            // float z = 0;
            ray r;
            struct vec3 p = vec3RandDisc(&rng);
            struct vec3 defocus_sample = vec3Add(center, vec3Add(vec3Scale(defocus_disk_u, p.x), vec3Scale(defocus_disk_v, p.y)));
            r.origin = (defocus_angle <= 0) ? center : defocus_sample;
            r.dir = vec3Sub(r.origin, dest);
            //vec3Sub((struct vec3){x, y, 0}, r.origin);
            struct vec3 c = (struct vec3){0, 0, 0};
            ray tmp = r;
            for(int sample = 0 ; sample < SAMPLES; sample++){
                tmp.dir = vec3Add(r.dir, (struct vec3){intervalRandf(0.0f, 0.01, &rng), intervalRandf(0.0f, 0.01f, &rng), 0});
                c = vec3Add(c, scatter(getHit(tmp), &rng, 0));
            }
            c = vec3Scale(c, 1.0f/SAMPLES);
            writeColor(c.x, c.y, c.z);
            //writeColor(r, g, b);
        }
    }
    return 0;
}