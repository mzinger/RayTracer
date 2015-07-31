/*
 * Primitive.h
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#ifndef PRIMITIVE_H_
#define PRIMITIVE_H_

#include "global.h"

class Primitive {
protected:
    Primitive(RGB & c, Material & m, mat4 modelToWorld); //Primitives are an abstract class

public:
    virtual ~Primitive();

    /**
     * Returns the point along the ray parameter t
     * where the ray intersects this primitive.
     * returns numeric_limits<float>::infinity();
     * if no intersection occurs. */
    virtual double intersect(Ray & ray)=0;

    /** sets "normal" to be a normal for the given position on the object. */
    virtual vec4 calculateNormal(vec4 & position)=0;

    void setColor(RGB & c);
    void setMaterial(Material & m);
    const RGB& getColor();
    const Material& getMaterial();

protected:
	mat4 _modelToWorld;
	mat4 _worldToModel;

private:
    RGB _c;
    Material _m;
};

class Sphere : public Primitive {
public:
    Sphere(double radius, RGB & c, Material & m, mat4 modelToWorld);

    double intersect(Ray & ray);
    vec4 calculateNormal(vec4 & position);

private:
    double _r;

};

class Triangle : public Primitive {
public:
    Triangle(vec3 a, vec3 b, vec3 c, RGB & col, Material & m, mat4 m2w);

    double intersect(Ray &ray);
    vec4 calculateNormal(vec4 & position);

private:
    vec3 verts[3];
};

#endif /* PRIMITIVE_H_ */
