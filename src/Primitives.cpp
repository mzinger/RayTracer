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
    Ray ray_t = ray;
    ray_t.transform(_worldToModel);
    // Check intersection of transformed ray with the sphere centered at origin
    float det = pow((ray_t.direction()*ray_t.start()), 2) - (ray_t.direction()*ray_t.direction())*(ray_t.start()*ray_t.start() - pow(_r,2));
    if (det < 0) return numeric_limits<float>::infinity();
    else det = sqrt(det);
    float base = -1*(ray_t.direction() * ray_t.start());
    if (base - det < 0) {
        return (base + det) / (ray_t.direction()*ray_t.direction());
    }
    return (base - det) / (ray_t.direction()*ray_t.direction());
}

//Calculates the normal for the given position on this sphere.
inline vec4 Sphere::calculateNormal(vec4 & position) {
    vec4 normal = (_worldToModel.transpose() * position) / _r;
    assert(normal*normal == 1);
    return normal;
}


Triangle::Triangle(vec3 a, vec3 b, vec3 c, RGB & col, Material & m, mat4 m2w) : Primitive(col,m,m2w) {
    verts[0] = a; verts[1] = b; verts[2] = c;
}

double Triangle::intersect(Ray & ray) {
    Ray ray_t = ray;
    ray_t.transform(_worldToModel);
    mat3 A((verts[0] - verts[1]), (verts[0] - verts[2]), ray_t.direction());
    A = A.transpose();
    float det_A = A.determinant();
    if (det_A <= 0) return numeric_limits<float>::infinity();
    mat3 B(verts[0] - vec3(ray_t.start()), verts[0] - verts[2], vec3(ray_t.direction()));
    B = B.transpose();
    mat3 C(verts[0] - verts[1], verts[0] - vec3(ray_t.direction()), vec3(ray_t.direction()));
    C = C.transpose();
    mat3 D(verts[0] - verts[1], verts[0] - verts[2], verts[0] - vec3(ray_t.start()));
    D = D.transpose();
    float t = D.determinant() / det_A;
    if (t < 0) return numeric_limits<float>::infinity();
    float gamma = C.determinant() / det_A;
    if (gamma < 0 || gamma > 1) return numeric_limits<float>::infinity();
    float beta = B.determinant() / det_A;
    if (beta < 0 || beta > 1 - gamma) return numeric_limits<float>::infinity();
    return t;
}

vec4 Triangle::calculateNormal(vec4 & position) {
    // normal is same at all positions.
    vec3 normal = (verts[1]- verts[0]) ^ (verts[2] - verts[0]);
    normal = _worldToModel.transpose() * normal;
    normal.normalize();
    return vec4(normal, 1);
}