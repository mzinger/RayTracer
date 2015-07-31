/*
 * Viewport.h
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#ifndef VIEWPORT_H_
#define VIEWPORT_H_

#include "global.h"

/**
 * Viewport holds all the information about our camera.
 * This includes how to sample across the viewport. This code should not
 * need to change.
 */
class Viewport {
public:
    Viewport(vec4 eye, vec4 LL, vec4 UL, vec4 LR, vec4 UR, int width, int height);
    virtual ~Viewport();

    /**
     * This will reset the sampler back to its initial state.
     */
    void resetSampler();
    /**
     * This ONLY fills in the (i, j) coordinates of the point according
     * to the pixels_wide and pixels_high settings
     */
    bool getSample(Sample & s);

    /**
     * This constructs the ray "r" to be a ray from the eye to the given sample p.
     */
    Ray createViewingRay(Sample & s);

    /**
     * Constructs a normalized vector pointing from the given position to the
     * camera.
     */
    vec4 getViewVector(vec4 & position);

    int getW();
    int getH();

private:
    vec4 _eye;
    vec4 _LL;
    vec4 _UL;
    vec4 _LR;
    vec4 _UR;

    int _pixelsWide;
    int _pixelsHigh;
    double _x;
    double _y;

	int _raysPerPixel;
	int _incPP;


};

#endif /* VIEWPORT_H_ */
