/*
 * Primitive.h
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#ifndef PRIMITIVE_H_
#define PRIMITIVE_H_

#include "global.h"

class Primitive;

// An axis aligned bounding box.
class Box {
public:
    Box () : _bottomLeft(vec3(0, 0, 0)), _topRight(vec3(0,0,0)) {};
    Box (vec3 bottom_left, vec4 top_right) : _bottomLeft(bottom_left), _topRight(top_right) {};
    Box(const Box& other) {
        _bottomLeft = other._bottomLeft;
        _topRight = other._topRight;
    }
    Box(const vector<Primitive*>& primitives);
    
    void transform(mat4 tansmat);  // in place transform the bounding box and take the bounding box of the transform
    vec3 bottomLeft() {
        return _bottomLeft;
    }
    vec3 topRight() {
        return _topRight;
    }
    Box combine(Box& other);
    bool intersect(Ray& ray);
protected:
    vec3 _bottomLeft;
    vec3 _topRight;
};

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
    virtual vec3 calculateNormal(vec4 & position)=0;
    /** Returns the bounding box of this primitive in world coordinates **/
    Box boundingBox() {
        return _boundingBox;
    }

    void setColor(RGB & c);
    void setMaterial(Material & m);
    const RGB& getColor();
    const Material& getMaterial();

protected:
	mat4 _modelToWorld;
	mat4 _worldToModel;
    Box _boundingBox;

private:
    RGB _c;
    Material _m;
};

class Sphere : public Primitive {
public:
    Sphere(double radius, RGB & c, Material & m, mat4 modelToWorld);

    double intersect(Ray & ray);
    vec3 calculateNormal(vec4 & position);
    Box boundingBox();

private:
    double _r;
    vec4 _center;

};

class Triangle : public Primitive {
public:
    Triangle(vec3 a, vec3 b, vec3 c, RGB & col, Material & m, mat4 m2w);

    double intersect(Ray &ray);
    vec3 calculateNormal(vec4 & position);
    Box boundingBox();

private:
    vec3 verts[3];
};

class BVHNode {
public:
    BVHNode(vector<Primitive*>& primitives, int AXIS);

    // Returns the primitive in the subtree rooted at this node that fist intersects a ray or NULL if no intersection.
    Primitive* intersect(Ray& r, double& t);

    Box boundingBox() {
        return _boundingBox;
    }

    BVHNode* _left_child;
    BVHNode* _right_child;
    Primitive* _primitive;  // if this is a leaf node.
    Box _boundingBox;
};

#endif /* PRIMITIVE_H_ */
