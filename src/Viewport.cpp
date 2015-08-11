/*
 * Viewport.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#include "Viewport.h"

Viewport::Viewport(vec4 eye, vec4 LL, vec4 UL, vec4 LR, vec4 UR, int width, int height, mat4 localToWorld) {
    vec3 a(eye);
    a[VZ] += DEPTH_OF_FIELD;
    a[VX] += 100000000.0;
    
    vec3 b(eye);
    b[VZ] += DEPTH_OF_FIELD;
    b[VX] -= 100000000.0;
    b[VY] += 50000000.0;
    
    vec3 c(eye);
    c[VZ] += DEPTH_OF_FIELD;
    c[VX] -= 100000000.0;
    c[VY] -= 50000000.0;
    
    RGB color(0,0,0);
    Material m;
    
    _focalPlane = new Triangle(a, b, c, color, m, NULL, NULL, localToWorld);
    
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
    s.setX(_x);
    s.setY(_y);
    _x += 1;
    if (_x > _pixelsWide - 1) {
        _x -= _pixelsWide;
        _y += 1;
        if (_y > _pixelsHigh - 1) {
            _y -= _pixelsHigh;
            _x = (rand() % 101) / 100.0;
            _y = (rand() % 101) / 100.0;
            _raysPerPixel--;
            if (_raysPerPixel == 0) {
                return false;
            }
        }
    }
    return true;
}

vector<Ray> Viewport::createViewingRays(Sample & s) {
    double u = 1.0 * s.x() / _pixelsWide;
    double u_tag = 1.0 - u;
    double v = 1.0 * s.y() / _pixelsHigh;
    double v_tag = 1.0 - v;
    
    vec4 p = (((_LL * v_tag) + (_UL * v)) * u_tag) + (((_LR * v_tag) + (_UR * v)) * u);
    
    vec4 direction = p - _eye;
    direction[VW] = 1;

    vec3 start(_eye);
    vec3 end(-direction);
    
    Ray viewRay = Ray(start, end, 0.1);
    
    vector<Ray> result;
    if (DEPTH_OF_FIELD) {
        double t = _focalPlane->intersect(viewRay);
        if (!t || t == std::numeric_limits<float>::infinity()) {
            cout << "ERROR: no intersection with focal plane " << s.x() << "," << s.y() << endl;
        
        }
        vec3 focal_point = start + t*end;
        for (int i = 0; i < RAYS_PER_PIXEL_SAMPLE; i++) {
            vec3 tempStart(start);
            tempStart[VX] += epsilon(CAMERA_LENS_SIZE);
            tempStart[VY] += epsilon(CAMERA_LENS_SIZE);
            tempStart[VZ] += epsilon(CAMERA_LENS_SIZE);
            vec3 tempEnd(-(focal_point - tempStart));
            Ray ray(tempStart, focal_point, 0.1);
            result.push_back(ray);
        }
    } else {
        result.push_back(viewRay);
    }
    return result;
}

vec4 Viewport::getViewVector(vec3 & pos) {
    return ((vec3)_eye - pos).normalize();
}

int Viewport::getW() { return _pixelsWide; }

int Viewport::getH() { return _pixelsHigh; }

