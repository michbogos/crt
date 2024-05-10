#ifndef TEXTURE
#define TEXTURE

#include"stb_image.h"

enum TextureType{
    TEXTURE_2D,
    TEXTURE_3D,
    TEXTURE_PERLIN,
    TEXTURE_CONST
};


struct Texture{
    enum TextureType type;
    float* data;
    int x;
    int y;
    int z;
    int channels;
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

struct vec3 sampleTexture(struct Texture* tex, struct vec3 coords){
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