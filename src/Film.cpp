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
    FreeImage_Initialise();
    _output = FreeImage_Allocate(_w, _h, 24);
    _data = new FloatRGB[_w * _h];
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
    RGBQUAD color;
    color.rgbRed = c[RED] * 255;
    color.rgbGreen = c[GREEN] * 255;
    color.rgbBlue = c[BLUE] * 255;
    FreeImage_SetPixelColor(_output, (int)s.x(), (int)s.y(), &color);
    _data[_w * (int)s.y() + (int)s.x()] = (FloatRGB) c;
    BindTexture();
    glutPostRedisplay();
    glutMainLoopEvent();
}

void Film::bakeAndSave() {
	FreeImage_Save(FIF_PNG, _output, _filename.c_str());
}