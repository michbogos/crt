#include<stdio.h>
#include"vec3.h"
#include"util.h"
#include"ray.h" 
#include"objects.h"

#define WIDTH 1920
#define HEIGHT 1080

point3 centers[] = {(point3){1,1,3}, (point3){1, 12, 4}};
float radii[] = {2, 8};

vec3 color(ray r){
    int hit = 0;
    struct hitRecord rec;
    rec.t = 1000.0f;
    for(int i = 0; i < 2; i++){
        struct hitRecord tmp;
        if(hitSphere(r, centers[i], radii[i], &tmp)){
            hit += 1;
            if(rec.t > tmp.t){
                rec = tmp;
            }
        }
    }
    if(hit){
        return vec3Add(vec3Scale(rec.normal, 128), (vec3){128, 128, 128});
    }
    else{
        vec3 u_dir = vec3Unit(r.dir);
        float a = 0.5f * (u_dir.y+1);
        return (vec3){a*128, a*225, a*255};
    }
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
    vec3 a = {0, 0, 0};
    vec3 b = {1, 1, 1};
    vec3 c = vec3Cross(a, b);
    float aspectRatio = (float)WIDTH/(float)HEIGHT;
    printf("P3\n%d %d\n255\n", WIDTH, HEIGHT);
    for(int  j = 0 ; j < HEIGHT; j++){
        for(int i = 0 ;i < WIDTH; i++){
            float x = ((float)i/(float)(WIDTH))*aspectRatio;
            float y = ((float)j/(float)(HEIGHT));
            float z = 0;
            ray r;
            r.origin = (vec3){0.5, 0.5, -0.5};
            r.dir = vec3Sub((vec3){x, y, 0}, r.origin);

            vec3 c = color(r);
            writeColor(c.x, c.y, c.z);
            //writeColor(r, g, b);
        }
    }
    return 0;
}