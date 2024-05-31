#ifndef TEXTURE
#define TEXTURE

#include"stb_image.h"

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


struct Texture{
    enum TextureType type;
    int offset;
    int x;
    int y;
    int z;
    int channels;
    float scale;
    float* base;
};

//Change base later in opencl
struct vec3 sampleTexture(struct Texture* tex, struct vec3 coords){
    int hi;
    float h;
    float cx;
    float cy;
    switch (tex->type)
    {
    case TEXTURE_CONST:
        return (struct vec3){*(tex->base+tex->offset+0), *(tex->base+tex->offset+1), *(tex->base+tex->offset+2)};
        break;
    
    case TEXTURE_2D:
        float trash;
        coords.x = modff(coords.x, &trash);
        coords.y = modff(coords.y, &trash);
        coords.z = modff(coords.z, &trash);
        unsigned int bytePerPixel = 3;
        float* pixelOffset = tex->base+tex->offset + (((int)(coords.x*tex->x) + tex->x * (int)(coords.y*tex->y)) * bytePerPixel);
        float r = pixelOffset[0];
        float g = pixelOffset[1];
        float b = pixelOffset[2];
        return (struct vec3){(float)r, (float)g, (float)b};
        break;
    
    case TEXTURE_PERLIN:
        {
           return (struct vec3){0.6, 0.0, 0.6};
        }
        break;
    case TEXTURE_NOISE:
        cx = roundf((coords.x/tex->scale))*tex->scale;
        cy = roundf((coords.y/tex->scale))*tex->scale;
        // cx = roundf((coords.z/tex->scale))*tex->scale;
        hi = (int)*(tex->base+tex->offset) + cx*374761393 + cy*668265263;
        hi = (hi^(hi >> 13))*1274126177;
        h = ((float)(hi^(hi >> 16)));
        h /= (float)RAND_MAX;
        return (struct vec3){h, h, h};
        break;
    case TEXTURE_CHECKER:
        return ((int)(floorf((1.0f/tex->scale)*coords.x))+
        (int)(floorf((1.0f/tex->scale)*coords.y))) % 2 == 0 ?
        (struct vec3){*(tex->base+tex->offset+0), *(tex->base+tex->offset+1), *(tex->base+tex->offset+2)} :
        (struct vec3){*(tex->base+tex->offset+3), *(tex->base+tex->offset+4), *(tex->base+tex->offset+5)};
        break;
    
    case TEXTURE_UV:
        return (struct vec3){coords.x, coords.y, coords.z};
    
    default:
        return (struct vec3){0, 1, 1};
        break;
    }
}



#endif