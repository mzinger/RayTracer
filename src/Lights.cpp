/*
 * Lights.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#include "Lights.h"


Light::Light() {
    RGB black(0,0,0);
    _illumination = black;
	_falloff = 0;
	_angularFalloff = 0;
	_deadDistance = 1;
}

Light::Light(RGB illumination) {
    _illumination = illumination;
}
Light::Light(RGB illumination, double falloff, double deadDistance) {
	_illumination = illumination;
		_falloff = falloff;
		_deadDistance = deadDistance;
}

Light::~Light() { }

const RGB & Light::getColor() {
    return _illumination;
}
const RGB Light::getColor(vec4 & p) {
    return _illumination;
}

void Light::setColor(RGB c) {
    _illumination = c;
}

AmbientLight::AmbientLight() {
    //intentionally empty
}

AmbientLight::AmbientLight(RGB illumination) : Light(illumination) {
    //intentionally empty
}

void AmbientLight::getIncidenceVector(vec4 & position, vec3 & returnValue) {
    throw "AMBIENT LIGHTS DO NOT HAVE A SENSE OF DIRECTION OR POSITION`";
}
Ray AmbientLight::getShadowRay(vec4 & position, bool & useDist) {
    throw "AMBIENT LIGHTS DO NOT HAVE A SENSE OF DIRECTION OR POSITION";
}


PointLight::PointLight(RGB illumination) : Light(illumination){
    //intentionally empty
}
PointLight::PointLight(RGB illumination, double falloff, double deadDistance) : 
	Light(illumination, falloff, deadDistance) {
    //intentionally empty
}

const RGB PointLight::getColor(vec4 & p) {
    float distance = ((vec3)_pos - (vec3)p).length();
    float dimming = 1 / pow(distance + _deadDistance, _falloff);
    return dimming * _illumination;
}
void PointLight::setPosition(vec4 pos) {
    _pos = pos;
}

void PointLight::getIncidenceVector(vec4 & position, vec3 & returnValue) {
    returnValue = vec3(_pos - position, VW);  // drop last component
    returnValue.normalize();
}

Ray PointLight::getShadowRay(vec4 & position, bool & useDist) {
    vec3 start(position, VW);
    vec3 end(_pos, VW);
    return Ray(start, end, 0.01);
}

vector<pair<Ray, double>> PointLight::GetShadowRays(vec4 & position, bool & useDist) {
    vector<pair<Ray, double>> result;
    if (POINT_LIGHT_SOURCE_SIZE > 0) {
        for (int i = 0; i < RAYS_PER_LIGHT_SOURCE; i++) {
            vec3 start(position, VW);
            vec3 end(_pos, VW);
            end[VX] += epsilon(POINT_LIGHT_SOURCE_SIZE);
            end[VY] += epsilon(POINT_LIGHT_SOURCE_SIZE);
            end[VZ] += epsilon(POINT_LIGHT_SOURCE_SIZE);
            Ray light_ray(start, end, 0.01);
            double t_max = light_ray.computeT(end);
            result.push_back(make_pair(light_ray, t_max));
        }
    } else {
        Ray shadow_ray = getShadowRay(position, useDist);
        result.push_back(make_pair(shadow_ray, shadow_ray.computeT(_pos)));
    }
    return result;
}



DirectionalLight::DirectionalLight(RGB illumination) : Light(illumination){
    //intentionally empty
}

void DirectionalLight::setDirection(vec4 dir) {
    _dir = dir;
    _dir[VW] = 0.0;
}

void DirectionalLight::getIncidenceVector(vec4 & position, vec3 & returnValue) {
    returnValue = vec3(-1*_dir, VW) ; // drop last component
    returnValue.normalize();

}
Ray DirectionalLight::getShadowRay(vec4 & position, bool & useDist) {
    vec3 start(position);
    vec3 end(-1 * _dir);
    return Ray(start, end, 0.1);
}
