#include<stdio.h>
#include<math.h>
#include"vec3.h"
#include"util.h"
#include"ray.h" 
#include"objects.h"
#include"pcg_basic.h"

#define WIDTH 512
#define HEIGHT 512
#define SAMPLES 20
#define DEPTH 10

enum matType{
    LAMBERT,
    METAL
};

struct materialInfo{
    enum matType type;
    struct vec3 color;
    float attenuation;
    int max_bounces;
};

struct materialInfo lambert = {.attenuation = 0.5, .max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL};
struct materialInfo metal = {.attenuation = 0.5, .max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL};

struct vec3 centers[] = {(struct vec3){0,0,3}, (struct vec3){1, 1.7, 2}};
float radii[] = {1, 1};
struct materialInfo mats[] = {(struct materialInfo){.attenuation = 0.5, .max_bounces=10, .color={0.7, 0.7, 1.0}, .type=LAMBERT},
                              (struct materialInfo){.attenuation = 0.5, .max_bounces=10, .color={0.1, 0.7, 1.0}, .type=METAL}};



pcg32_random_t rng;

//Maybe add sky as a seperate object and material
struct hitRecord getHit(ray r){
    int hit = 0;
    struct hitRecord rec;
    rec.t = 1000000.0f;
    for(int i = 0; i < 2; i++){
        struct hitRecord tmp;
        if(hitSphere(r, centers[i], radii[i], &tmp)){
            hit += 1;
            if(rec.t > tmp.t && tmp.t > 0.00001f){
                rec = tmp;
                rec.id = i;
            }
        }
    }
    rec.r = r;
    return rec;
}
//Maybe pass a pointer to avoid large memory consumption
//Returns a color
struct vec3 scatter(struct hitRecord rec, pcg32_random_t* rng, int depth){
    ray new_ray;
    struct vec3 dir;
    struct materialInfo info = mats[rec.id];

    if(depth > info.max_bounces){
        return (struct vec3){0, 0, 0};
    }
    if(rec.t > 99999.9f){
        struct vec3 u_dir = vec3Unit(rec.r.dir);
        float a = 0.5f * (u_dir.y+1);
        return (struct vec3){a*0.5, a*0.7, a*1.0};
    }

    switch (info.type)
    {
    case LAMBERT:
        dir = vec3Add(rec.normal, vec3RandHemisphere(rec.normal, rng));
        new_ray = (ray){rayAt(rec.r, rec.t), dir};
        break;
    
    case METAL:
        dir = vec3Reflect(rec.r.dir, rec.normal);
        new_ray = (ray){rayAt(rec.r, rec.t), dir};
        break;
    
    default:
        dir = vec3Add(rec.normal, vec3RandHemisphere(rec.normal, rng));
        new_ray = (ray){rayAt(rec.r, rec.t), dir};
        break;
    }
    //New ray

    //Get a hit record
    struct hitRecord hit = getHit(new_ray);
    struct vec3 color = scatter(hit, rng, depth+1);
    color.x *= info.color.x;
    color.y *= info.color.y;
    color.z *= info.color.z;
    return color;
}

// struct vec3 color(ray r, int depth){
//     if(depth > DEPTH){
//         return (struct vec3){0.0, 0.0, 0.0};
//     }
//     int hit = 0;
//     struct hitRecord rec;
//     rec.t = 100000.0f;
//     for(int i = 0; i < 3; i++){
//         struct hitRecord tmp;
//         if(hitSphere(r, centers[i], radii[i], &tmp)){
//             hit += 1;
//             if(rec.t > tmp.t && tmp.t > 0.0001f){
//                 rec = tmp;
//             }
//         }
//     }
//     if(hit){
//         ray new_ray = scatter()
//         return vec3Scale(color(new_ray, depth+1), 0.5);
//     }
//     struct vec3 u_dir = vec3Unit(r.dir);
//     float a = 0.5f * (vec3Unit(r.dir).y+1);
//     return (struct vec3){a*0.5, a*0.8, a};

//     // float t = hitSphere(r, (point3){0.5, 0.5, 3}, 2, &rec);
//     // if(t > 0){
//     //     vec3 N = vec3Unit(vec3Sub(rayAt(r, t), (vec3){0, 0, -1}));
//     //     return (vec3){(N.x+1)*128, (N.y+1)*128, (N.z+1)*128};
//     // }
//     // vec3 u_dir = vec3Unit(r.dir);
//     // float a = 0.5f * (u_dir.y+1);
//     // return (vec3){a*128, a*225, a*255};
// }

int main(){
     pcg32_srandom_r(&rng, 42u, 54u); // Constant seed

    float aspectRatio = (float)WIDTH/(float)HEIGHT;
    printf("P3\n%d %d\n255\n", WIDTH, HEIGHT);
    for(int  j = 0 ; j < HEIGHT; j++){
        for(int i = 0 ;i < WIDTH; i++){
            float x = ((float)i/(float)(WIDTH))*aspectRatio;
            float y = ((float)j/(float)(HEIGHT));
            float z = 0;
            ray r;
            r.origin = (struct vec3){0.5, 0.5, -0.5};
            r.dir = vec3Sub((struct vec3){x, y, 0}, r.origin);
            struct vec3 c = (struct vec3){0, 0, 0};
            for(int sample = 0 ; sample < SAMPLES; sample++){
                ray tmp = r;
                tmp.dir = vec3Add(tmp.dir, (struct vec3){intervalRandf(0.0f, 0.001f, &rng), intervalRandf(0.0f, 0.001f, &rng), 0});
                c = vec3Add(c, scatter(getHit(tmp), &rng, 0));
            }
            c = vec3Scale(c, 1.0f/SAMPLES);
            writeColor(c.x, c.y, c.z);
            //writeColor(r, g, b);
        }
    }
    return 0;
}