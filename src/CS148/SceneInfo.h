/*
 * SceneInfo.h
 *
 * These basic holder structs are used to return data to you
 *
 *  Created on: Feb 19, 2009
 *      Author: jima
 */

#ifndef SCENE_INFO_H_
#define SCENE_INFO_H_

#include "algebra3.h"

enum {
    MAT_MA,     //ambient
    MAT_MD,     //diffuse reflection
    MAT_MS,     //specular reflection
    MAT_MSP,    //specular reflection phong exponent, the "roughness"
    MAT_MSM,    //metalicity,
    MAT_MT,     //transparency for refracted rays
    MAT_MTN,    //refractive index of the material
    MAT_NUM_COEFFICIENTS
}; // phong coefficients

enum { 
    FRUS_LEFT, FRUS_RIGHT, FRUS_BOTTOM, FRUS_TOP, FRUS_NEAR, FRUS_FAR
}; // frustum sides

enum {
    LIGHT_AMBIENT, LIGHT_DIRECTIONAL, LIGHT_POINT, LIGHT_SPOT
}; // light types


struct MaterialInfo {
    double k[MAT_NUM_COEFFICIENTS]; //Storage for the material coefficients
    RGB color;

    MaterialInfo() {}
    MaterialInfo(RGB color, const double ka, const double kd, const double ks, const double ksp, const double ksm, const double kt, const double ktn)
        : color(color)
    {
        k[MAT_MA] = ka;
        k[MAT_MD] = kd;
        k[MAT_MS] = ks;
        k[MAT_MSP] = ksp;
        k[MAT_MSM] = ksm;
        k[MAT_MT] = kt;
        k[MAT_MTN] = ktn;
    }
};


struct LightInfo {
    int type;
    RGB color;
    double falloff;
    double angularFalloff;
    double deadDistance;

    LightInfo() {}
    LightInfo(int type, RGB color, double falloff = 2, double angularFalloff = .1, double deadDistance = .1)
        : type(type), color(color), falloff(falloff), angularFalloff(angularFalloff), deadDistance(deadDistance)
    {}
};

struct CameraInfo {
    int perspective;
    double sides[6];

    CameraInfo() {}
    CameraInfo(int perspective, double l, double r, double b, double t, double n=1, double f=100)
        : perspective(perspective)
    {
        sides[FRUS_LEFT] = l;
        sides[FRUS_RIGHT] = r;
        sides[FRUS_BOTTOM] = b;
        sides[FRUS_TOP] = t;
        sides[FRUS_NEAR] = n;
        sides[FRUS_FAR] = f;
    }
};


#endif /* SCENE_INFO_H_ */