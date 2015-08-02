/*
 * Viewport.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#include "Viewport.h"

Viewport::Viewport(vec4 eye, vec4 LL, vec4 UL, vec4 LR, vec4 UR, int width, int height) {
    _eye = eye;	//You should use this for the viewing ray
    _LL = LL;
    _UL = UL;
    _LR = LR;
    _UR = UR;
    _pixelsWide = width;
    _pixelsHigh = height;
	_raysPerPixel = RAYS_PER_PIXEL;
	_incPP = sqrt((float)_raysPerPixel);
}

Viewport::~Viewport() {

}

void Viewport::resetSampler() {
    _x = _y = 1.0 / (_incPP*2); //Starts off at the center of a pixel.
}

/**
 * This ONLY fills in the (i, j) coordinates of the point according
 * to the pixels_wide and pixels_high settings
 */
bool Viewport::getSample(Sample & s) {
    s.setX(random()% IMAGE_WIDTH);
    s.setY(random()% IMAGE_HEIGHT);
    /*
    // TODO(matan): Add validations, and return false if needed.
    s.setX(int((s.x() - (_UL[VX])) * _pixelsWide / (_UR[VX] - _UL[VX])));
    s.setX(int((s.y() - (_LL[VY])) * _pixelsHigh / (_UL[VY] - _LL[VY])));*/
    return true;
}

Ray Viewport::createViewingRay(Sample & s) {
    vec3 start(s.x(), s.y(), 0);
    vec3 end(_eye);
    return Ray(start, end, 0.0);
}

vec4 Viewport::getViewVector(vec4 & pos) {
    return (pos - _eye).normalize();
}

int Viewport::getW() { return _pixelsWide; }

int Viewport::getH() { return _pixelsHigh; }

