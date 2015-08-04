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
    _center = vec4(0, 0, 0, 1);  // always at origin wrt a transformation.
    // Take bounding box of untransformed sphere, transform it and then return its bounding box
    Box box(vec3(-_r, -_r, -_r), vec3(_r, _r, _r));
    box.transform(_modelToWorld);
    _boundingBox = box;
}


//Checks for intersection with the given ray
double Sphere::intersect(Ray & ray) {
    Ray ray_t = ray;
    ray_t.transform(_worldToModel);

    // Check intersection of transformed ray with the sphere centered at origin
    float det = pow((ray_t.direction()*(ray_t.start() - _center)), 2) - (ray_t.direction()*ray_t.direction())*(((ray_t.start()-_center)*(ray_t.start()-_center)) - pow(_r,2));
    if (det < 0) return numeric_limits<float>::infinity();
    else det = sqrt(det);
    float base = -1*(ray_t.direction() * (ray_t.start() - _center));
    if (base - det < 0) {
        return (base + det) / (ray_t.direction()*ray_t.direction());
    }
    return (base - det) / (ray_t.direction()*ray_t.direction());
}

//Calculates the normal for the given position on this sphere.
inline vec3 Sphere::calculateNormal(vec4 & position) {
    vec3 position_t = _worldToModel*position;
    vec3 normal = vec3(_worldToModel.transpose() * vec4(position_t / _r, 0), VW);
    return normal.normalize();
}

Triangle::Triangle(vec3 a, vec3 b, vec3 c, RGB & col, Material & m, mat4 m2w) : Primitive(col,m,m2w) {
    verts[0] = a; verts[1] = b; verts[2] = c;
    // Take bounding box of untransformed triangle, transform it and then return its bounding box
    double min_x = MIN(MIN(verts[0][0], verts[1][0]), verts[2][0]);
    double min_y = MIN(MIN(verts[0][1], verts[1][1]), verts[2][1]);
    double min_z = MIN(MIN(verts[0][2], verts[1][2]), verts[2][2]);
    double max_x = MAX(MAX(verts[0][0], verts[1][0]), verts[2][0]);
    double max_y = MAX(MAX(verts[0][1], verts[1][1]), verts[2][1]);
    double max_z = MAX(MAX(verts[0][2], verts[1][2]), verts[2][2]);
    Box box(vec3(min_x, min_y, min_z), vec3(max_x, max_y, max_z));
    box.transform(_modelToWorld);
    _boundingBox = box;
}

double Triangle::intersect(Ray & ray) {
    Ray ray_t = ray;
    ray_t.transform(_worldToModel);
    mat3 A((verts[0] - verts[1]), (verts[0] - verts[2]), vec3(ray_t.direction(), VW)); // drop W comp of dir
    A = A.transpose();
    float det_A = A.determinant();
    mat3 B(verts[0] - vec3(ray_t.start()), verts[0] - verts[2], vec3(ray_t.direction(), VW));
    B = B.transpose();
    mat3 C(verts[0] - verts[1], verts[0] - vec3(ray_t.start()), vec3(ray_t.direction(), VW));
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

vec3 Triangle::calculateNormal(vec4 & position) {
    // normal is same at all positions.
    vec3 normal = (verts[1]- verts[0]) ^ (verts[2] - verts[0]);
    normal = _worldToModel.transpose() * vec4(normal, 0);
    return (vec3(normal, VW)).normalize();
}

void Box::transform(mat4 trans) {
    // explicitly create all 8 points, apply transformation, then recomoute BB
    vec3 p1(bottomLeft);
    vec3 p2(topRight[0], bottomLeft[1], bottomLeft[2]);
    vec3 p3(topRight[0], topRight[1], bottomLeft[2]);
    vec3 p4(bottomLeft[0], topRight[1], bottomLeft[2]);
    vec3 p5(topRight[0], bottomLeft[1], topRight[2]);
    vec3 p6(topRight[0], topRight[1], topRight[2]);
    vec3 p7(bottomLeft[0], topRight[1], topRight[2]);
    vec3 p8(topRight);
    p1 = trans*p1; p2 = trans*p2; p3 = trans*p3; p4 = trans*p4;
    p5 = trans*p5; p6 = trans*p6; p7 = trans*p7; p8 = trans*p8;
    double min_x = MIN(MIN(MIN(MIN(MIN(MIN(MIN(p1[0], p2[0]),p3[0]),p4[0]),p5[0]),p6[0]),p7[0]),p8[0]);
    double min_y = MIN(MIN(MIN(MIN(MIN(MIN(MIN(p1[1], p2[1]),p3[1]),p4[1]),p5[1]),p6[1]),p7[1]),p8[1]);
    double min_z = MIN(MIN(MIN(MIN(MIN(MIN(MIN(p1[2], p2[2]),p3[2]),p4[2]),p5[2]),p6[2]),p7[2]),p8[2]);
    double max_x = MAX(MAX(MAX(MAX(MAX(MAX(MAX(p1[0], p2[0]),p3[0]),p4[0]),p5[0]),p6[0]),p7[0]),p8[0]);
    double max_y = MAX(MAX(MAX(MAX(MAX(MAX(MAX(p1[1], p2[1]),p3[1]),p4[1]),p5[1]),p6[1]),p7[1]),p8[1]);
    double max_z = MAX(MAX(MAX(MAX(MAX(MAX(MAX(p1[2], p2[2]),p3[2]),p4[2]),p5[2]),p6[2]),p7[2]),p8[2]);
    bottomLeft = vec3(min_x, min_y, min_z);
    topRight = vec3(max_x, max_y, max_z);
}