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
    _verticesHaveNormals = false;
    _normal = (verts[2]- verts[0]) ^ (verts[1] - verts[0]);
    //_normal.normalize();
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
    if (!_verticesHaveNormals) {
        // normal is same at all positions.
        vec3 normal = _worldToModel.transpose() * vec4(_normal, 0);
        return vec3(normal, VW).normalize();
    }
    // Use barycentric interpolation to compute normals.
    vec3 v0 = verts[1] - verts[0];
    vec3 v1 = verts[2] - verts[0];
    vec3 v2 = vec3(_worldToModel*position) - verts[0];
    
    // Compute dot products
    double dot00 = v0 * v0;
    double dot01 = v0 * v1;
    double dot02 = v0 * v2;
    double dot11 = v1 * v1;
    double dot12 = v1 * v2;
    
    // Compute barycentric coordinates
    double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    double beta = (dot11 * dot02 - dot01 * dot12) * invDenom;
    double gamma = (dot00 * dot12 - dot01 * dot02) * invDenom;
    double alpha = 1 - (beta + gamma);
    assert(alpha >= 0); assert(beta >= 0); assert(gamma >= 0);
    assert(alpha <= 1); assert(beta <= 1); assert(gamma <= 1);
    vec3 normal = _normal;
    if (alpha >= 0 && alpha <= 1 && beta >=0 && beta <=1 && gamma >=0 && gamma <=1) {
        normal = alpha * _vertexNormals[0] + beta * _vertexNormals[1] + gamma * _vertexNormals[2];
    }
    normal = _worldToModel.transpose() * vec4(normal, 0);
    return vec3(normal, VW).normalize();
}

Box::Box (const vector<Primitive*>& primitives) {
    assert(primitives.size() >= 1);
    Box bb(primitives[0]->boundingBox().bottomLeft(), primitives[0]->boundingBox().topRight());
    for (int i = 1; i < primitives.size(); ++i) {
        Box bb2 = primitives[i]->boundingBox();
        bb = bb.combine(bb2);
    }
    _bottomLeft = bb.bottomLeft();
    _topRight = bb.topRight();
}

void Box::transform(mat4 trans) {
    // explicitly create all 8 points, apply transformation, then recomoute BB
    vec3 p1(_bottomLeft);
    vec3 p2(_topRight[0], _bottomLeft[1], _bottomLeft[2]);
    vec3 p3(_topRight[0], _topRight[1], _bottomLeft[2]);
    vec3 p4(_bottomLeft[0], _topRight[1], _bottomLeft[2]);
    vec3 p5(_topRight[0], _bottomLeft[1], _topRight[2]);
    vec3 p6(_topRight[0], _topRight[1], _topRight[2]);
    vec3 p7(_bottomLeft[0], _topRight[1], _topRight[2]);
    vec3 p8(_topRight);
    p1 = trans*p1; p2 = trans*p2; p3 = trans*p3; p4 = trans*p4;
    p5 = trans*p5; p6 = trans*p6; p7 = trans*p7; p8 = trans*p8;
    double min_x = MIN(MIN(MIN(MIN(MIN(MIN(MIN(p1[0], p2[0]),p3[0]),p4[0]),p5[0]),p6[0]),p7[0]),p8[0]);
    double min_y = MIN(MIN(MIN(MIN(MIN(MIN(MIN(p1[1], p2[1]),p3[1]),p4[1]),p5[1]),p6[1]),p7[1]),p8[1]);
    double min_z = MIN(MIN(MIN(MIN(MIN(MIN(MIN(p1[2], p2[2]),p3[2]),p4[2]),p5[2]),p6[2]),p7[2]),p8[2]);
    double max_x = MAX(MAX(MAX(MAX(MAX(MAX(MAX(p1[0], p2[0]),p3[0]),p4[0]),p5[0]),p6[0]),p7[0]),p8[0]);
    double max_y = MAX(MAX(MAX(MAX(MAX(MAX(MAX(p1[1], p2[1]),p3[1]),p4[1]),p5[1]),p6[1]),p7[1]),p8[1]);
    double max_z = MAX(MAX(MAX(MAX(MAX(MAX(MAX(p1[2], p2[2]),p3[2]),p4[2]),p5[2]),p6[2]),p7[2]),p8[2]);
    _bottomLeft = vec3(min_x, min_y, min_z);
    _topRight = vec3(max_x, max_y, max_z);
}

Box Box::combine(Box& other) {
    // explicitly create all 8 points, apply transformation, then recomoute BB
    double min_x = MIN(_bottomLeft[0], other.bottomLeft()[0]);
    double min_y = MIN(_bottomLeft[1], other.bottomLeft()[1]);
    double min_z = MIN(_bottomLeft[2], other.bottomLeft()[2]);
    double max_x = MAX(_topRight[0], other.topRight()[0]);
    double max_y = MAX(_topRight[1], other.topRight()[1]);
    double max_z = MAX(_topRight[2], other.topRight()[2]);
    return Box(vec3(min_x, min_y, min_z), vec3(max_x, max_y, max_z));
}

bool Box::intersect(Ray& ray) {
    vec3 dir = vec3(ray.direction(), VW);
    vec3 start = ray.start();
    double a = 1/dir[0], b = 1/dir[1], c = 1/dir[2];
    double t_x_min, t_x_max, t_y_min, t_y_max, t_z_min, t_z_max;
    if (a >= 0) {
        t_x_min = a * (_bottomLeft[0] - start[0]);
        t_x_max = a * (_topRight[0] - start[0]);
    } else {
        t_x_min = a * (_topRight[0] - start[0]);
        t_x_max = a * (_bottomLeft[0] - start[0]);
    }
    if (b >= 0) {
        t_y_min = b * (_bottomLeft[1] - start[1]);
        t_y_max = b * (_topRight[1] - start[1]);
    } else {
        t_y_min = b * (_topRight[1] - start[1]);
        t_y_max = b * (_bottomLeft[1] - start[1]);
    }
    if (c >= 0) {
        t_z_min = c * (_bottomLeft[2] - start[2]);
        t_z_max = c * (_topRight[2] - start[2]);
    } else {
        t_z_min = c * (_topRight[2] - start[2]);
        t_z_max = c * (_bottomLeft[2] - start[2]);
    }
    if ((t_x_min > t_y_max) || (t_x_max < t_y_min) ||
        (t_y_min > t_z_max) || (t_y_max < t_z_min) ||
        (t_x_min > t_z_max) || (t_x_max < t_z_min)) {
            return false;
    }
    return true;
}

BVHNode::BVHNode(vector<Primitive*>& primitives, int AXIS) {
    _primitive = NULL;
    int N = primitives.size();
    assert (N >=1);
    if (N==1) {
        _primitive = primitives[0];
        _left_child = NULL;
        _right_child = NULL;
        _boundingBox = _primitive->boundingBox();
    } else {
        Box bb = Box(primitives);
        double midpoint = (bb.bottomLeft()[AXIS] + bb.topRight()[AXIS]) / 2;
        vector<Primitive*> left_list, right_list;
        for (Primitive* p : primitives) {
            if ((p->boundingBox().bottomLeft()[AXIS] - midpoint) *
                (p->boundingBox().topRight()[AXIS] - midpoint) >= 0) {
                // Completely on one side
                if (p->boundingBox().bottomLeft()[AXIS] - midpoint > 0) {
                    right_list.push_back(p);
                } else {
                    left_list.push_back(p);
                }
            } else {
                // randomly put on any side
                if (rand() % 2 == 0) {
                    right_list.push_back(p);
                } else {
                    left_list.push_back(p);
                }
            }
        }
        _left_child = NULL; _right_child = NULL;
        if (left_list.size() > 0 && right_list.size() > 0) {
            _left_child = new BVHNode(left_list, (AXIS + 1)%3);
            _right_child = new BVHNode(right_list, (AXIS + 1)%3);
            _boundingBox = _right_child->boundingBox();
            _boundingBox = _left_child->boundingBox().combine(_boundingBox);
        } else if (left_list.size() > 0 && right_list.size() == 0) {
            _left_child = new BVHNode(left_list, (AXIS + 1)%3);
            _boundingBox = _left_child->boundingBox();
        } else if (left_list.size() == 0 && right_list.size() > 0) {
            _right_child = new BVHNode(right_list, (AXIS + 1)%3);
            _boundingBox = _right_child->boundingBox();
        } else {
            // Cannnot happen.
            assert(false);
        }
    }
}

Primitive* BVHNode::intersect(Ray& ray, double& t) {
    if (_primitive != NULL) {
        // Leaf node
        double curr_t = _primitive->intersect(ray);
        if (curr_t < std::numeric_limits<double>::max() && curr_t > ray.getMinT()) {
            t = curr_t;
            return _primitive;
        }
    }
    if (!_boundingBox.intersect(ray)) return NULL;
    double t_l, t_r;
    Primitive* left_result = NULL;
    Primitive* right_result = NULL;
    if (_left_child != NULL) {
        left_result = _left_child->intersect(ray, t_l);
    }
    if (_right_child != NULL) {
        right_result = _right_child->intersect(ray, t_r);
    }
    if (left_result == NULL && right_result == NULL) {
        t = std::numeric_limits<double>::max();
        return NULL;
    } else if (left_result == NULL) {
        t = t_r;
        return right_result;
    } else if (right_result == NULL) {
        t = t_l;
        return left_result;
    }
    // Both subtrees have interection. Pick the one in front.
    t = MIN(t_l, t_r);
    return t_l < t_r ? left_result : right_result;
}