/*
 * Primitive.h
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#ifndef PRIMITIVE_H_
#define PRIMITIVE_H_

#include "global.h"
struct Texture;
class Primitive;
class PerlinNoise;

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
    Primitive(RGB & c, Material & m, Texture* texture, Texture* bumpTexture, mat4 modelToWorld); //Primitives are an abstract class
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
    virtual RGB getColor(vec3 position);
    const Material& getMaterial();
protected:
	mat4 _modelToWorld;
	mat4 _worldToModel;
    Box _boundingBox;
    RGB _c;
    Texture* _texture;
    Texture* _bumpTexture;
private:
    Material _m;
};

class Sphere : public Primitive {
public:
    Sphere(double radius, RGB & c, Material & m, Texture* t, Texture* bumpTexture, PerlinNoise* noise, mat4 modelToWorld);

    double intersect(Ray & ray);
    vec3 calculateNormal(vec4 & position);
    Box boundingBox();
    RGB getColor(vec3 position);

private:
    double _r;
    vec4 _center;
    PerlinNoise* _noise;
};

class Triangle : public Primitive {
    friend class World;
public:
    Triangle(vec3 a, vec3 b, vec3 c, RGB & col, Material & m, Texture* t, Texture* bumpTexture, mat4 m2w);
    Triangle(vec3 a, vec3 n1, vec3 b, vec3 n2, vec3 c, vec3 n3, RGB & col, Material & m, Texture* t, Texture* bumpTexture, mat4 m2w);
    Triangle(vec3 a, vec2 t1, vec3 b, vec2 t2, vec3 c, vec2 t3, RGB & col, Material & m, Texture* t, Texture* bumpTexture, mat4 m2w);
    Triangle(vec3 a, vec2 t1, vec3 n1, vec3 b, vec2 t2, vec3 n2, vec3 c, vec2 t3, vec3 n3, RGB & col, Material & m, Texture* t, Texture* bumpTexture, mat4 m2w);

    double intersect(Ray &ray);
    RGB getColor(vec3 position);
    vec3 calculateNormal(vec4 & position);
    Box boundingBox();

private:
    vec3 verts[3];
    vec3 _vertexNormals[3];
    vec2 _textureCoord[3];
    vec3 _normal;
    bool _verticesHaveNormals;
    bool _verticesHaveTexture;
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
