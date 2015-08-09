#ifndef TEXTURE_H_
#define TEXTURE_H_
#endif

#include "algebra3.h"

struct Texture {
    int _width;
    int _height;
    FloatRGB* _data;
    
    Texture(string filename);
};
