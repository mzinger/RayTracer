#include "texture.h"

#include <FreeImage.h>

Texture::Texture(string filename) {
    FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename.c_str());
    FIBITMAP *bmp = FreeImage_Load(fif, filename.c_str());
    _width = FreeImage_GetWidth(bmp);
    _height =  FreeImage_GetHeight(bmp);
    _data = new FloatRGB[_width*_height];
    RGBQUAD color;
    for (int x = 0; x < _width; ++x) {
        for (int y = 0; y < _height; ++y) {
            FreeImage_GetPixelColor(bmp, x, y, &color);
            RGB rgb(color.rgbRed, color.rgbGreen, color.rgbBlue);
            rgb /= 255;
            _data[y*_width+x] = rgb;
        }
    }
}

FloatRGB Texture::getValue(double u, double v) {
    return _data[int(_width*v + u)];
}