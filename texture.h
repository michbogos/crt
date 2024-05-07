#ifndef TEXTURE
#define TEXTURE

#include"stb_image.h"

enum TextureType{
    TEXTURE_2D,
    TEXTURE_2D_HDR,
    TEXTURE_3D
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
    return tex;
}



#endif