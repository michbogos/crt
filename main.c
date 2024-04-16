#include<stdio.h>
#include<math.h>
#include"vec3.h"
#include"util.h"
#include"ray.h" 
#include"objects.h"

#define WIDTH 512
#define HEIGHT 512
#define SAMPLES 20
#define DEPTH 10

struct vec3 centers[] = {(struct vec3){1,1,3}, (struct vec3){1, 12, 4}, (struct vec3){0, 1.5, 2}};
float radii[] = {2, 8, 1};

struct vec3 color(ray r, int depth){
    if(depth > DEPTH){
        return (struct vec3){0.0, 0.0, 0.0};
    }
    int hit = 0;
    struct hitRecord rec;
    rec.t = 100000.0f;
    for(int i = 0; i < 3; i++){
        struct hitRecord tmp;
        if(hitSphere(r, centers[i], radii[i], &tmp)){
            hit += 1;
            if(rec.t > tmp.t && tmp.t > 0.0001f){
                rec = tmp;
            }
        }
    }
    if(hit){
        struct vec3 dir = vec3Add(rec.normal, vec3RandHemisphere(rec.normal));
        ray new_ray = (ray){rayAt(r, rec.t), dir};
        return vec3Scale(color(new_ray, depth+1), 0.5);
    }
    struct vec3 u_dir = vec3Unit(r.dir);
    float a = 0.5f * (vec3Unit(r.dir).y+1);
    return (struct vec3){a*0.5, a*0.8, a};

    // float t = hitSphere(r, (point3){0.5, 0.5, 3}, 2, &rec);
    // if(t > 0){
    //     vec3 N = vec3Unit(vec3Sub(rayAt(r, t), (vec3){0, 0, -1}));
    //     return (vec3){(N.x+1)*128, (N.y+1)*128, (N.z+1)*128};
    // }
    // vec3 u_dir = vec3Unit(r.dir);
    // float a = 0.5f * (u_dir.y+1);
    // return (vec3){a*128, a*225, a*255};
}

int main(){
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
                tmp.dir = vec3Add(tmp.dir, (struct vec3){intervalRandf(0.0f, 0.001f), intervalRandf(0.0f, 0.001f), 0});
                c = vec3Add(c, color(tmp, 0));
            }
            c = vec3Scale(c, 1.0f/SAMPLES);
            writeColor(c.x, c.y, c.z);
            //writeColor(r, g, b);
        }
    }
    return 0;
}