/*
 * Film.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#include "Film.h"

Film::Film(int w, int h, string filename) {
	_w = w;
	_h = h;
	_filename = filename;
    _counter = 0;
    FreeImage_Initialise();
    _output = FreeImage_Allocate(_w, _h, 24);
    _data = new FloatRGB[_w * _h];
    _numSamples = new int[_w * _h];
    for (int i = 0; i < _w*_h; ++i) _numSamples[i] =  0;
}

Film::~Film() {
    delete _output;
    delete _data;
    FreeImage_DeInitialise();
}

void Film::BindTexture() {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMAGE_WIDTH, IMAGE_HEIGHT, 0,
                 GL_RGB, GL_FLOAT, data());
}

/**
 * Here we stick the "saving" of the image.
 * In our first case, this involves using OpenGL to display the pixels.
 */
void Film::expose(RGB c, Sample & s) {
    int index = _w * (int)s.y() + (int)s.x();
    _data[index] = (_data[index]*_numSamples[index] + (FloatRGB) c) / (_numSamples[index]+1);
    ++_numSamples[index];
    _counter = (_counter + 1) % 5000;
    if (_counter == 0 && OPENGL_RENDER) {  // only render every 100th exposure
        BindTexture();
        glutPostRedisplay();
        glutMainLoopEvent();
    }
}

void Film::bakeAndSave() {
    for (int i = 0; i < _w*_h; ++i) {
        const FloatRGB& c = _data[i];
        int y = i / _w;
        int x = i % _w;
        RGBQUAD color;
        color.rgbRed = c[RED] * 255;
        color.rgbGreen = c[GREEN] * 255;
        color.rgbBlue = c[BLUE] * 255;
        FreeImage_SetPixelColor(_output, x, y, &color);
    }
	FreeImage_Save(FIF_PNG, _output, _filename.c_str());
}