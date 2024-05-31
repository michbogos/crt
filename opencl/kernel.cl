// RANDOM FUNCTIONS

struct __attribute__((packed)) pcg_state_setseq_64 {    // Internals are *Private*.
    unsigned long state;             // RNG state.  All values are possible.
    unsigned long inc;               // Controls which RNG sequence (stream) is
                                // selected. Must *always* be odd.
};
typedef struct pcg_state_setseq_64 pcg32_random_t;

unsigned int pcg32_random_r(__global pcg32_random_t* rng)
{
    unsigned long oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    unsigned int xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    unsigned int rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

float unitRandf(__global pcg32_random_t* rng){ //0-1 range
    return ((float)pcg32_random_r(rng))/((float)2147483647.0f);
}

//VECTOR FUNCTIONS

struct __attribute__ ((packed)) vec3
{
    float x;
    float y;
    float z;
};

struct vec3 vec3Add(struct vec3 a, struct vec3 b){
    struct vec3 c;
    c.x = a.x+b.x;
    c.y = a.y+b.y;
    c.z = a.z+b.z;
    return c;
}

struct vec3 vec3Sub(struct vec3 a, struct vec3 b){
    struct vec3 c;
    c.x = a.x-b.x;
    c.y = a.y-b.y;
    c.z = a.z-b.z;
    return c;
}

struct vec3 vec3Scale(struct vec3 a, float fac){
    struct vec3 c;
    c.x = a.x*fac;
    c.y = a.y*fac;
    c.z = a.z*fac;
    return c;
}

float vec3Dot(struct vec3 a, struct vec3 b){
    return a.x*b.x+a.y*b.y+a.z*b.z;
}

struct vec3 vec3Cross(struct vec3 a, struct vec3 b){
    struct vec3 c;
    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;
    return c;
}

  float vec3Mag(struct vec3 a){
    return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

  float vec3Mag2(struct vec3 a){
    return a.x*a.x + a.y*a.y + a.z*a.z;
}

struct vec3 vec3Unit(struct vec3 a){
    return vec3Scale(a, 1.0f/vec3Mag(a));
}

  float vec3Dist(struct vec3 a, struct vec3 b){
    struct vec3 d = vec3Sub(a, b);
    return sqrt(d.x*d.x + d.y*d.y+ d.z*d.z);
}

struct vec3 vec3Reflect(struct vec3 a, struct vec3 normal){
    return vec3Sub(a, vec3Scale(normal, 2*vec3Dot(a,normal)));
}

struct vec3 vec3Refract(struct vec3 a, struct vec3 normal, float ior){
    float cos_theta = fmin(
            vec3Dot(vec3Scale(a, -1), normal), 1.0f
        );
    struct vec3 r_perp = vec3Scale(vec3Add(a, vec3Scale(normal, cos_theta)), ior);
    struct vec3 r_par = vec3Scale(normal, -sqrt(fabs(1.0-vec3Mag2(r_perp))));
    return vec3Add(r_perp, r_par);
}

 struct vec3 vec3RandUnit(__global pcg32_random_t* rng){
    return vec3Unit((struct vec3){unitRandf(rng), unitRandf(rng), unitRandf(rng)});
}

struct vec3 vec3RandHemisphere(struct vec3 normal, __global pcg32_random_t* rng){
    while(1){
        struct vec3 v = vec3RandUnit(rng);
        if(vec3Mag(v) <= 1.0){
            if(vec3Dot(normal, v)>0){
                return v;
            }
            else{
                return vec3Scale(v, -1);
            }
        }
    }
}

struct vec3 vec3RandDisc(__global pcg32_random_t* rng){
    while(1){
        struct vec3 p = (struct vec3){unitRandf(rng)*2-1, unitRandf(rng)*2-1, 0};
            if(vec3Mag2(p)<1){
                return p;
            }
        }
}

//RAY FUNCTIONS

struct __attribute__((packed)) ray{
    struct vec3 origin;
    struct vec3 dir;
};

struct vec3 rayAt(struct ray r, float t){
    return vec3Add(r.origin, vec3Scale(r.dir, t));
}

//CAMERA FUNCTIONS

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

struct Camera initCamera(struct Camera c, int width, int height){
    struct Camera cam = c;
    cam.aspect_ratio = (float)width/(float)height;
    cam.h = tan(cam.fov/2);

    cam.focal_length = vec3Mag(vec3Sub(cam.pos, cam.look_at));
    cam.focus = 0.01f;
    cam.defocus_angle = -1;
    cam.defocus_radius = cam.focus*tan(cam.defocus_angle/2);

    cam.viewport_height = 2*cam.h*cam.focal_length;
    cam.viewport_width = cam.viewport_height*cam.aspect_ratio;
    cam.w = vec3Unit(vec3Sub(cam.pos, cam.look_at));
    cam.u = vec3Unit(vec3Cross(cam.camera_up, cam.w));
    cam.v = vec3Cross(cam.w, cam.u);

    cam.viewport_u = vec3Scale(cam.u, cam.viewport_width);
    cam.viewport_v = vec3Scale(cam.v, cam.viewport_height);
    cam.du = vec3Scale(cam.viewport_u, 1.0f/width);
    cam.dv = vec3Scale(cam.viewport_v, 1.0f/height);
    cam.defocus_disk_u = vec3Scale(cam.u, cam.defocus_radius);
    cam.defocus_disk_v = vec3Scale(cam.v, cam.defocus_radius);
    cam.top_left = vec3Sub(vec3Sub(vec3Sub(cam.pos, vec3Scale(cam.w, -cam.focal_length)), vec3Scale(cam.viewport_u, 0.5)), vec3Scale(cam.viewport_v, 0.5));
    return cam;
}

struct ray getRay(struct Camera cam, int i, int j){
    struct vec3 dest = vec3Add(cam.pos, vec3Add(vec3Add(cam.top_left, vec3Scale(cam.du, i)), vec3Scale(cam.dv, j)));
    // float y = ((float)j/(float)(HEIGHT))*h*2;
    // float z = 0;
    struct ray r;
    r.dir = vec3Sub(r.origin, dest);

    return r;
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

int matInvert(__global float* m)
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

struct vec3 vec3matmul(struct vec3 a, __global float* mat){
    struct vec3 res;
    res.x = a.x*mat[0]+a.y*mat[1]+a.z*mat[2]+mat[3];
    res.y = a.x*mat[4]+a.y*mat[5]+a.z*mat[6]+mat[7];
    res.z = a.x*mat[8]+a.y*mat[9]+a.z*mat[10]+mat[11];
    return res;
}

//TEXTURE FUNCTIONS

enum TextureType{
    TEXTURE_2D,
    TEXTURE_3D,
    TEXTURE_PERLIN,
    TEXTURE_NOISE,
    TEXTURE_CONST,
    TEXTURE_CHECKER,
    TEXTURE_UV,
    TEXTURE_NORM
};


struct __attribute__((packed))  Texture{
    enum TextureType type;
    __global float* data;
    int x;
    int y;
    int z;
    int channels;
    float scale;
};

struct vec3 sampleTexture(__global struct Texture* tex, struct vec3 coords){
    int hi;
    float h;
    float cx;
    float cy;
    switch (tex->type)
    {
    case TEXTURE_CONST:
        return (struct vec3){tex->data[0], tex->data[1], tex->data[2]};
        break;
    
    case TEXTURE_2D:
    {
        double trash = 0;
        coords.x = modf(coords.x, &trash);
        coords.y = modf(coords.y, &trash);
        coords.z = modf(coords.z, &trash);
        unsigned int bytePerPixel = 3;
        __global float* pixelOffset = tex->data + (((int)(coords.x*tex->x) + tex->x * (int)(coords.y*tex->y)) * bytePerPixel);
        float r = pixelOffset[0];
        float g = pixelOffset[1];
        float b = pixelOffset[2];
        return (struct vec3){(float)r, (float)g, (float)b};
    }
        break;
    
    case TEXTURE_PERLIN:
        return (struct vec3){0.0, 1.0, 1.0};
        break;
    case TEXTURE_NOISE:{
        cx = round((coords.x/tex->scale))*tex->scale;
        cy = round((coords.y/tex->scale))*tex->scale;
        // cx = roundf((coords.z/tex->scale))*tex->scale;
        hi = (int)tex->data[0] + cx*374761393 + cy*668265263;
        hi = (hi^(hi >> 13))*1274126177;
        h = ((float)(hi^(hi >> 16)));
        h /= 2147483647.0f;
        return (struct vec3){h, h, h};
        }
        break;
    case TEXTURE_CHECKER:
        return ((int)(floor((1.0f/tex->scale)*coords.x))+
        (int)(floor((1.0f/tex->scale)*coords.y))) % 2 == 0 ?
        (struct vec3){tex->data[0], tex->data[1], tex->data[2]} :
        (struct vec3){tex->data[3], tex->data[4], tex->data[5]};
        break;
    
    case TEXTURE_UV:
        return (struct vec3){coords.x, coords.y, coords.z};
    
    default:
        return (struct vec3){0, 1, 1};
        break;
    }
}

//OBJECT FUNCTIONS

enum matType{
    LAMBERT,
    METAL,
    DIELECTRIC,
    EMISSIVE
};

struct __attribute__((packed))  materialInfo{
    enum matType type;
    struct vec3 color;
    struct vec3 emissiveColor;
    __global struct Texture* texture;
    __global struct Texture* normal;
    float fuzz;
    float ior;
    int max_bounces;
};

struct __attribute__((packed))  hitRecord{
    struct ray r;
    float t;
    int id;
    struct vec3 normal;
    struct vec3 uv;
    struct materialInfo mat;
    int front_face;
};

enum ObjectType{
    SPHERE, AABB, QUAD, TRI
};

struct __attribute__((packed)) Hittable{
    enum ObjectType type;
    int matIndex;
    int id;
    unsigned int offset;
    __global float* transform_matrix;
    __global float* inverse_matrix;
};

struct __attribute__((packed))  Sphere{
    struct vec3 center;
    float radius;
};

struct __attribute__((packed))  AABB{
    float x0;
    float x1;
    float y0;
    float y1;
    float z0;
    float z1;
    struct Hittable* object;
};

struct __attribute__((packed))  Quad{
    struct vec3 p;
    struct vec3 u;
    struct vec3 v;
    struct vec3 w;
    struct vec3 n;
    struct vec3 normal;
    float D;
};

struct __attribute__((packed))  Triangle{
    struct vec3 a;
    struct vec3 b;
    struct vec3 c;
    struct vec3 norma;
    struct vec3 normb;
    struct vec3 normc;
    struct vec3 uva;
    struct vec3 uvb;
    struct vec3 uvc;
};

struct __attribute__((packed))  Mesh{
    int index;
    int size;
    int matIdx;
};

// int intervalOverlap(float x0, float x1, float y0, float y1){
//     if(x0 > x1){
//         float tmp = x0;
//         x0 = x1;
//         x1 = tmp;
//     }
//     if(y0 > y1){
//         float tmp = y0;
//         y0 = y1;
//         y1 = tmp;
//     }
//     return x0 <= y1 && y0 <= x1;
// }

// int intersectAABB(ray r, __global struct AABB* aabb){
//     float tx0 = (aabb->x0-r.origin.x)/r.dir.x;
//     float tx1 = (aabb->x1-r.origin.x)/r.dir.x;
//     float ty0 = (aabb->y0-r.origin.y)/r.dir.y;
//     float ty1 = (aabb->y1-r.origin.y)/r.dir.y;
//     float tz0 = (aabb->z0-r.origin.z)/r.dir.z;
//     float tz1 = (aabb->z1-r.origin.z)/r.dir.z;

//     return intervalOverlap(tx0, tx1, ty0, ty1) && intervalOverlap(ty0, ty1, tz0, tz1) && intervalOverlap(tx0, tx1, tz0, tz1);
// }

// int hitSphere(ray r, struct Sphere s, struct hitRecord* rec){
//     struct vec3 oc = vec3Sub(s.center, r.origin);
//     float a = vec3Dot(r.dir, r.dir);
//     float b =  -2.0 * vec3Dot(r.dir, oc);
//     float c = vec3Dot(oc, oc)-s.radius*s.radius;
//     float discriminant = b*b-4*a*c;
//     if(discriminant < 0.0f){
//         return 0;
//     }
//     float t  = (-b - sqrt(discriminant) ) / (2.0*a);
//     rec->t = t;
//     rec->normal= vec3Scale(vec3Sub(rayAt(r, t), s.center), 1.0f/s.radius);
//     rec->front_face = vec3Dot(r.dir, rec->normal) < 0.0f ? 1 : -1;

//     struct vec3 u_dir = vec3Unit(rec->normal);
//     rec->uv = (struct vec3){0.5f+atan2(u_dir.z, u_dir.x)/2/3.1415926f, 0.5f+asin(u_dir.y)/3.1415926f, 0.0f};
//     return 1;
// }

// int hitQuad(ray r, struct Quad quad, struct hitRecord* rec){
//     float denom = vec3Dot(quad.normal, r.dir);

//     if(fabs(denom)<1e-8f){
//         return 0;
//     }
//     float t = (quad.D - vec3Dot(quad.normal, r.origin)) / denom;

//     struct vec3 hitpt = vec3Sub(rayAt(r, t), quad.p);
//     float alpha = vec3Dot(quad.w, vec3Cross(hitpt, quad.v));
//     float beta =  vec3Dot(quad.w, vec3Cross(quad.u, hitpt));

//     if(!(0.0f < alpha && alpha < 1.0f) || !(0.0f < beta && beta < 1.0f)){
//         return 0;
//     }

//     rec->normal = quad.normal;
//     rec->t = t;
//     rec->front_face = vec3Dot(r.dir, rec->normal) < 0.0f ? 1 : -1;
//     rec->uv = (struct vec3){alpha, beta, 0.0f};
//     return 1;
// }

// int hitTri(ray r, struct Triangle tri, struct hitRecord* rec){
//     struct vec3 v1v0 = vec3Sub(tri.b, tri.a);
//     struct vec3 v2v0 = vec3Sub(tri.c, tri.a);
//     struct vec3 rov0 = vec3Sub(r.origin, tri.a);
 
//     struct vec3 n = vec3Cross( v1v0, v2v0);
//     struct vec3 q = vec3Cross( rov0, r.dir);
//     float d = 1.0/vec3Dot(  n, r.dir );
//     float u =   d*vec3Dot( vec3Scale(q, -1.0f), v2v0);
//     float v =   d*vec3Dot(  q, v1v0 );
//     float t =   d*vec3Dot( vec3Scale(n, -1.0f), rov0);
//     float w = 1.0f-u-v;

//     rec->t = t;

//     if(t<0.00000001){
//         return 0;
//     }

//     rec->r = r;

//     rec->normal = vec3Unit(vec3Add(vec3Scale(tri.norma, w), vec3Add(vec3Scale(tri.normb, u), vec3Scale(tri.normc, v))));

//     rec->uv = vec3Add(vec3Scale(tri.uva, w), vec3Add(vec3Scale(tri.uvb, u), vec3Scale(tri.uvc, v)));
//     rec->front_face = vec3Dot(r.dir, rec->normal) > 0.0f ? 1 : -1;
 
//     return (u<0.0 || v<0.0 || (u+v)>1.0) ? 0 : 1;
// }

// //BVH FUNCTIONS

// struct __attribute__((packed, aligned(4))) LBvh{
//     int object;
//     int left;
//     int right;
//     int axis;
// };

// int traverseLBvh(__global struct Hittable* objects, __private struct Hittable* vec, __global struct LBvh* nodes, __global struct AABB* boxes, ray r){
//     int current_node = 0;

//     int to_visit[2048];
//     int to_visit_size = 1;
//     int to_visit_available = 2048;
//     int beginning = 0;
//     int vecIdx = 0;
//     to_visit[0] = 0;

//     while(beginning < to_visit_size){
//         for(int i = beginning; i < to_visit_size; i++){
//             int node = to_visit[i];
//             // assert(node != -1);
//             beginning += 1;
//             if(nodes[node].object > -1){
//                 vec[vecIdx] = objects[nodes[node].object];
//                 vecIdx ++;
//             }
//             int left = nodes[node].left;
//             int right = nodes[node].right;
//             if(left != -1){
//                 if(intersectAABB(r, boxes+left)){
//                     // if(to_visit_size+1 == to_visit_available){
//                     //     to_visit_available *= 2;
//                     //     int* tmp = malloc(to_visit_size*sizeof(int));
//                     //     memcpy(tmp, to_visit, to_visit_size);
//                     //     free(to_visit);
//                     //     to_visit = tmp;
//                     // }
//                     to_visit[to_visit_size] = left;
//                     to_visit_size++;
//                 }
//             }
//             if(right != -1){
//             if(intersectAABB(r, boxes+right)){
//                 // if(to_visit_size+1 == to_visit_available){
//                 //     to_visit_available *= 2;
//                 //     int* tmp = malloc(to_visit_size*sizeof(int));
//                 //     memcpy(tmp, to_visit, to_visit_size);
//                 //     free(to_visit);
//                 //     to_visit = tmp;
//                 // }
//                 to_visit[to_visit_size] = right;
//                 to_visit_size++;
//             }
//         }
//     }
// }
//     return vecIdx;
// }


// //WORLD FUNCTIONS

// struct __attribute__((packed))  World{
//     __global struct materialInfo* materials;
//     __global struct Hittable* objects;
//     __global struct Bvh* tree;
//     __global struct LBvh* lbvh_nodes;
//     __global struct AABB* boxes;
//     __global struct Texture* envMap;
// };

// //Maybe add sky as a seperate object and material
// struct hitRecord getHit(ray r, struct World world){
//     int hit = 0;
//     struct hitRecord rec;
//     rec.t = 1000000.0f;
//     rec.r = r;
//     struct Hittable hittables[256];
//     int hittablesSize = 0;
//     hittablesSize = traverseLBvh(world.objects, hittables, world.lbvh_nodes,world.boxes, r);
//     //traverseBvh(&hittables, world.tree, r);
//     for(int i = 0; i < hittablesSize; i++){
//         struct hitRecord tmp;
//         //Transformed ray
//         ray tmpr = r;
//         struct vec3 point = vec3Add(tmpr.origin, tmpr.dir);
//         tmpr.origin = hittables[i].transform_matrix != (void*)(0) ? vec3matmul(tmpr.origin, hittables[i].inverse_matrix) : tmpr.origin;
//         point = hittables[i].transform_matrix != (void*)(0) ? vec3matmul(point, hittables[i].inverse_matrix) : point;
//         tmpr.dir = vec3Sub(point, tmpr.origin);
//         switch (hittables[i].type)
//         {
//             case SPHERE:
//             {
//                 struct Sphere s = *((__global struct Sphere*)hittables[i].data);
//                 if(hitSphere(tmpr, s, &tmp)){
//                     if(rec.t > tmp.t && tmp.t > 0.00001f){
//                         hit += 1;
//                         rec = tmp;
//                         rec.mat = world.materials[hittables[i].matIndex];
//                         rec.r = r;
//                     }
//                 }
//             }
//                 break;
//             case QUAD:
//             {
//                 struct Quad q = *((__global struct Quad*)hittables[i].data);
//                 if(hitQuad(r, q, &tmp)){
//                     if(rec.t > tmp.t && tmp.t > 0.00001f){
//                         hit += 1;
//                         rec = tmp;
//                         rec.mat = world.materials[hittables[i].matIndex];
//                     }
//                 }
//             }
//                 break;
//             case TRI:{
//                 struct Triangle tri = *((__global struct Triangle*)hittables[i].data);
//                 //Think of ray as two points
//                 if(hitTri(tmpr, tri, &tmp)){
//                     if(rec.t > tmp.t && tmp.t > 0.00001f){
//                         hit += 1;
//                         rec = tmp;
//                         rec.mat = world.materials[hittables[i].matIndex];
//                         rec.normal = hittables[i].transform_matrix != (void*)(0) ? vec3Unit(vec3Sub(vec3matmul(rec.normal, hittables[i].transform_matrix), vec3matmul((struct vec3){0, 0, 0}, hittables[i].transform_matrix))) : rec.normal;
//                         rec.r = r;
//                     }
//                 }
//             }
//                 break;
//         default:
//             printf("Default case\n");
//             break;
//         }
//     }
//     rec.mat = hit? rec.mat : world.materials[0];
//     return rec;
// }

// //MATERIAL FUNCTIONS

// struct vec3 linearScatter(struct hitRecord rec, struct World world, __global pcg32_random_t* rng, int depth){
//     ray new_ray;
//     struct materialInfo info;
//     struct vec3 color = (struct vec3){1, 1, 1};
//     struct hitRecord hit = rec;
//     for(int i = 0; i <= depth; i++){
//         info = hit.mat;
//         if(i == depth){
//             color = (struct vec3){0, 0, 0};
//             break;
//         }
//         if(hit.t > 999999.9f){
//             struct vec3 u_dir = vec3Unit(hit.r.dir);
//             float u = 0.5f+atan2(u_dir.z, u_dir.x)/2/3.1415926;
//             float v = 0.5f+asin(u_dir.y)/3.1415926;

//             // unsigned int bytePerPixel = 3;
//             // float* pixelOffset = env + (((int)(u*w) + w * (int)(v*h)) * bytePerPixel);
//             // float r = pixelOffset[0];
//             // float g = pixelOffset[1];
//             // float b = pixelOffset[2];

//             struct vec3 c = sampleTexture(world.envMap, (struct vec3){u, v, 0.0f});
//             color.x *= c.x;
//             color.y *= c.y;
//             color.z *= c.z;
//             break;
//         }

//         struct vec3 texColor = sampleTexture(info.texture, hit.uv);
//         color.x *= texColor.x;
//         color.y *= texColor.y;
//         color.z *= texColor.z;

//         color = vec3Add(color, info.emissiveColor);

//         struct vec3 normal = hit.normal;

//         if(info.normal != (void*)(0)){
//             struct vec3 z = normal;
//             struct vec3 y = vec3Unit(vec3Cross(normal, vec3Add(normal, vec3Scale(vec3RandHemisphere(normal, rng), 0.01f))));
//             struct vec3 x = vec3Unit(vec3Cross(z, y));
//             struct vec3 texNormal = sampleTexture(info.normal,  hit.uv);
//             float nx = vec3Dot(texNormal, x);
//             float ny = vec3Dot(texNormal, y);
//             float nz = vec3Dot(texNormal, z);
//             normal.x = nx;
//             normal.y = ny;
//             normal.z = nz;
//         }

//         switch (info.type){
//             case LAMBERT:
//                 new_ray = (ray){rayAt(hit.r, hit.t), vec3Add(normal, vec3RandHemisphere(normal, rng))};
//                 break;
            
//             case METAL:
//                 new_ray = (ray){rayAt(hit.r, hit.t), vec3Add(vec3Unit(vec3Reflect(hit.r.dir, normal)), vec3Scale(vec3RandUnit(rng), info.fuzz))};
//                 break;
//             case DIELECTRIC:{
//                 float ior = hit.front_face ? 1.0f/info.ior : info.ior;
//                 struct vec3 udir = vec3Unit(hit.r.dir);

//                 float cos_theta = fmin(vec3Dot(normal, vec3Scale(udir, -1)), 1.0f);
//                 float sin_theta = sqrt(1.0f-cos_theta*cos_theta);

//                 float r0 = (1 - ior) / (1 + ior);
//                 r0 = r0*r0;
//                 float reflectance = r0 + (1-r0)*pow((1 - cos_theta),5);

//                 struct vec3 refracted = ((ior*sin_theta > 1.0f) || (reflectance > unitRandf(rng))) ? vec3Reflect(udir, normal) : vec3Refract(udir, normal, ior);
//                 new_ray = (ray){rayAt(hit.r, hit.t), refracted};
//                 }
//                 break;
            
//             default:
//                 new_ray = (ray){rayAt(hit.r, hit.t), vec3Add(normal, vec3RandHemisphere(normal, rng))};
//                 break;
//         }
//         hit = getHit(new_ray, world);
//     }
//     return color;
// }


// __kernel void vadd(
//     __global struct vec3* a,                                                  
//     __global struct vec3* b,                                                 
//     __global struct vec3* c,                                                 
//     const unsigned int count){                                                                     
//         int i = get_global_id(0);                                          
//         if(i < count)                                                       
//         c[i] = vec3Cross(a[i], b[i]);
//         printf("Thread %d:\n a.x: %f a.y: %f a.z: %f\nb.x: %f b.y: %f b.z:%f\nc.x: %f c.y: %f c.z:%f\n", i, a[i].x, a[i].y, a[i].z,b[i].x, b[i].y, b[i].z,c[i].x, c[i].y, c[i].z);
// }

__kernel void getObj(__global float* image, int count){
    int i = get_global_id(0);
    struct Camera cam = {.camera_up=(struct vec3){0, 1, 0}, .look_at=(struct vec3){0, 0, 0}, .pos=(struct vec3){5, 5, 5}, .fov=1.5};
    cam = initCamera(cam, 1024, 1024);
    if(i<count){
        struct ray r = getRay(cam, i/1024, i/1024);
        image[3*i+0] = fabs(r.dir.z);
        image[3*i+1] = fabs(r.dir.z);
        image[3*i+2] = fabs(r.dir.z);
        // switch(arr[i].type){
        //     case TRI:{
        //         __global struct Triangle* t = (__global struct Triangle*)(data+arr[i].offset);
        //         printf("Thread: %d, ObjectType: %d, DataPoint: %f\n", i, arr[i].type, t->a.z);
        //         break;
        //     }
        //     default:
        //     printf("Thread: %d, ObjectType: OTHER\n", i);
        // }
    }
}