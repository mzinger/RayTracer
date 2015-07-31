/*
 * Lights.h
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#ifndef LIGHTS_H_
#define LIGHTS_H_

#include "global.h"

class Light {
protected:

    Light();
    Light(RGB illumination);
	Light(RGB illumination, double falloff, double deadDistance);
    RGB _illumination;
	double _falloff;
	double _angularFalloff;
	double _deadDistance;

public:

    virtual ~Light();

    virtual const RGB & getColor();
	virtual const RGB getColor(vec4 & p);
    void setColor(RGB c);

    /**
     * Returns the normalized vector describing the direction of light falling
     * onto given position. This is used in the shading calculation.
     */
    virtual void getIncidenceVector(vec4 & position, vec4 & returnValue)=0;

	virtual Ray getShadowRay(vec4 & position, bool & useDist)=0;
	
};

class AmbientLight : public Light {
public:
    AmbientLight();
    AmbientLight(RGB illumination);
    void getIncidenceVector(vec4 & position, vec4 & returnValue);
	Ray getShadowRay(vec4 & position, bool & useDist);
};

class PointLight : public Light {
public:
    PointLight(RGB illumination);
	PointLight(RGB illumination, double falloff, double deadDistance);
	const RGB getColor(vec4 & p);
    void setPosition(vec4 pos);
    void getIncidenceVector(vec4 & position, vec4 & returnValue);
	Ray getShadowRay(vec4 & position, bool & useDist);
private:
    vec4 _pos;
};

class DirectionalLight : public Light {
public:
    DirectionalLight(RGB illumination);
    void setDirection(vec4 dir);
    void getIncidenceVector(vec4 & position, vec4 & returnValue);
	Ray getShadowRay(vec4 & position, bool & useDist);
private:
    vec4 _dir;
};

#endif /* LIGHTS_H_ */
