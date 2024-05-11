#ifndef TEXTURE
#define TEXTURE

#include"stb_image.h"

enum TextureType{
    TEXTURE_2D,
    TEXTURE_3D,
    TEXTURE_PERLIN,
    TEXTURE_NOISE,
    TEXTURE_CONST,
    TEXTURE_CHECKER
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

struct Texture texChecker(float scale, struct vec3 color1, struct vec3 color2){
    struct Texture tex;
    tex.type = TEXTURE_CHECKER;
    tex.data = malloc(6*sizeof(float));
    tex.data[0] = color1.x;
    tex.data[1] = color1.y;
    tex.data[2] = color1.z;
    tex.data[3] = color2.x;
    tex.data[4] = color2.y;
    tex.data[5] = color2.z;
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
        {
            float cx0 = floorf((coords.x/tex->scale))*tex->scale;
            float cy0 = floorf((coords.y/tex->scale))*tex->scale;
            float cx1 = ceilf((coords.x/tex->scale))*tex->scale;
            float cy1 = ceilf((coords.y/tex->scale))*tex->scale;

            int rand0 = (int)tex->data[0] + cx0*374761393 + cy0*668265263;
            rand0 = (rand0^(rand0 >> 13))*1274126177;
            rand0 = (rand0^(rand0 >> 16));

            int rand1 = (int)tex->data[0] + cx0*374761393 + cy1*668265263;
            rand1 = (rand1^(rand1 >> 13))*1274126177;
            rand1 = (rand1^(rand1 >> 16));

            int rand2 = (int)tex->data[0] + cx1*374761393 + cy0*668265263;
            rand2 = (rand2^(rand2 >> 13))*1274126177;
            rand2 = (rand2^(rand2 >> 16));

            int rand3 = (int)tex->data[0] + cx1*374761393 + cy1*668265263;
            rand3 = (rand3^(rand3 >> 13))*1274126177;
            rand3 = (rand3^(rand3 >> 16));

            int rand0x = rand0 + tex->scale*374761393+tex->scale*668265263;
            rand0x = (rand0x^(rand0x >> 13))*1274126177;
            rand0x = (rand0x^(rand0x >> 16));
            int rand0y = rand0 + tex->scale*374761393*3.1415926+tex->scale*668265263;
            rand0y = (rand0y^(rand0y >> 13))*1274126177;
            rand0y = (rand0y^(rand0y >> 16));

            int rand1x = rand1 + tex->scale*374761393+tex->scale*668265263;
            rand1x = (rand1x^(rand1x >> 13))*1274126177;
            rand1x = (rand1x^(rand1x >> 16));
            int rand1y = rand1 + tex->scale*374761393*3.1415926+tex->scale*668265263;
            rand1y = (rand1y^(rand1y >> 13))*1274126177;
            rand1y = (rand1y^(rand1y >> 16));

            int rand2x = rand2 + tex->scale*374761393+tex->scale*668265263;
            rand2x = (rand2x^(rand2x >> 13))*1274126177;
            rand2x = (rand2x^(rand2x >> 16));
            int rand2y = rand2 + tex->scale*374761393*3.1415926+tex->scale*668265263;
            rand2y = (rand2y^(rand2y >> 13))*1274126177;
            rand2y = (rand2y^(rand2y >> 16));

            int rand3x = rand3 + tex->scale*374761393+tex->scale*668265263;
            rand3x = (rand3x^(rand3x >> 13))*1274126177;
            rand3x = (rand3x^(rand3x >> 16));
            int rand3y = rand3 + tex->scale*374761393*3.1415926+tex->scale*668265263;
            rand3y = (rand3y^(rand3y >> 13))*1274126177;
            rand3y = (rand3y^(rand3y >> 16));

            h = ((float)(rand0^(rand0 >> 16)));
            h /= (float)RAND_MAX;
            return (struct vec3){h, h, h};
        }
        break;
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
    case TEXTURE_CHECKER:
        return ((int)(floorf((1.0f/tex->scale)*coords.x))+
        (int)(floorf((1.0f/tex->scale)*coords.y))) % 2 == 0 ?
        (struct vec3){tex->data[0], tex->data[1], tex->data[2]} :
        (struct vec3){tex->data[3], tex->data[4], tex->data[5]};
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