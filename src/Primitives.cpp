/*
 * Primitive.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#include "Primitives.h"

Primitive::Primitive(RGB & c, Material & m, mat4 modelToWorld) {
    _c = c;
    _m = m;
    _modelToWorld = modelToWorld;
    _worldToModel = modelToWorld.inverse();
}

Primitive::~Primitive() {
}

void Primitive::setColor(RGB & c) {
    _c = c;
}

void Primitive::setMaterial(Material & m) {
    _m = m;
}

const RGB& Primitive::getColor() {
    return _c;
}
const Material& Primitive::getMaterial() {
    return _m;
}


Sphere::Sphere(double radius, RGB & c, Material & m, mat4 m2w): Primitive(c,m,m2w) {
    _r = radius;
}


//Checks for intersection with the given ray
double Sphere::intersect(Ray & ray) {
	//YOUR CODE HERE
    IMPLEMENT_ME(__FILE__,__LINE__);
}

//Calculates the normal for the given position on this sphere.
inline vec4 Sphere::calculateNormal(vec4 & position) {
	//YOUR CODE HERE
    IMPLEMENT_ME(__FILE__,__LINE__);
}


Triangle::Triangle(vec3 a, vec3 b, vec3 c, RGB & col, Material & m, mat4 m2w) : Primitive(col,m,m2w) {
    verts[0] = a; verts[1] = b; verts[2] = c;
}


//*
double Triangle::intersect(Ray & ray) {
	//OPTIONAL EXTRA CREDIT YOUR CODE HERE
    IMPLEMENT_ME(__FILE__,__LINE__);
}
// */

vec4 Triangle::calculateNormal(vec4 & position) {
	//OPTIONAL EXTRA CREDIT YOUR CODE HERE
    IMPLEMENT_ME(__FILE__,__LINE__);
}