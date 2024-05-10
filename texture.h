#ifndef TEXTURE
#define TEXTURE

#include"stb_image.h"

enum TextureType{
    TEXTURE_2D,
    TEXTURE_3D,
    TEXTURE_PERLIN,
    TEXTURE_NOISE,
    TEXTURE_CONST
};


struct Texture{
    enum TextureType type;
    float* data;
    int x;
    int y;
    int z;
    int channels;
    float scale;
};

struct Texture texFromFile(const char * filename){
    struct Texture tex;
    int x;
    int y;
    int ch;
    tex.data = stbi_loadf(filename, &x, &y, &ch, 3);
    tex.x = x;
    tex.y = y;
    tex.channels = ch;
    tex.type = TEXTURE_2D;
    return tex;
}

struct Texture texConst(struct vec3 color){
    struct Texture tex;
    tex.type = TEXTURE_CONST;
    tex.data = malloc(3*sizeof(float));
    tex.data[0] = color.x;
    tex.data[1] = color.y;
    tex.data[2] = color.z;
    return tex;
}

struct Texture texPerlin(float scale, float seed){
    struct Texture tex;
    tex.type = TEXTURE_PERLIN;
    tex.data = malloc(sizeof(float));
    tex.data[0] = seed;
    tex.scale = scale;
    return tex;
}

struct Texture texNoise(float scale, float seed){
    struct Texture tex;
    tex.type = TEXTURE_NOISE;
    tex.data = malloc(sizeof(float));
    tex.data[0] = seed;
    tex.scale = scale;
    return tex;
}

struct vec3 sampleTexture(struct Texture* tex, struct vec3 coords){
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
        unsigned int bytePerPixel = 3;
        float* pixelOffset = tex->data + (((int)(coords.x*tex->x) + tex->x * (int)(coords.y*tex->y)) * bytePerPixel);
        float r = pixelOffset[0];
        float g = pixelOffset[1];
        float b = pixelOffset[2];
        return (struct vec3){(float)r, (float)g, (float)b};
        break;
    
    case TEXTURE_PERLIN:
        break;
        // int h = (tex->data[0]) + coords.x*374761393 + coords.y*668265263;
        // h = (float)(hi^(hi >> 13))*1274126177;
        // return (struct vec3){(float)(h^(h >> 16)), (float)(h^(h >> 16)), (float)(h^(h >> 16))};
        // break;
    case TEXTURE_NOISE:
        cx = roundf((coords.x/tex->scale))*tex->scale;
        cy = roundf((coords.y/tex->scale))*tex->scale;
        // cx = roundf((coords.z/tex->scale))*tex->scale;
        hi = (int)tex->data[0] + cx*374761393 + cy*668265263;
        hi = (hi^(hi >> 13))*1274126177;
        h = ((float)(hi^(hi >> 16)));
        h /= (float)RAND_MAX;
        return (struct vec3){h, h, h};
        break;
    
    default:
        return (struct vec3){0, 1, 1};
        break;
    }
}

void freeTexture(struct Texture* tex){
    free(tex->data);
    return;
}



#endif