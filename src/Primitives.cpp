/*
 * Primitive.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#include "Primitives.h"
#include "CS148/PerlinNoise.h"
#include "CS148/Texture.h"

bool DOUBLE_EQ(double a, double b) {
    return (a - b < 0.0001);
}

Primitive::Primitive(RGB & c, Material & m, Texture* texture, Texture* bumpTexture, mat4 modelToWorld) {
    _c = c;
    _texture = texture;
    _bumpTexture = bumpTexture;
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

RGB Primitive::getColor(vec3 position) {
    return _c;
}

const Material& Primitive::getMaterial() {
    return _m;
}

/****************************************************************
 *													   *
 *		    Sphere                                             *
 *											   		   *
 ****************************************************************/

Sphere::Sphere(double radius, RGB & c, Material & m, Texture* t, Texture* bt, PerlinNoise* noise, mat4 m2w): Primitive(c,m,t,bt,m2w) {
    _r = radius;
    _center = vec4(0, 0, 0, 1);  // always at origin wrt a transformation.
    // Take bounding box of untransformed sphere, transform it and then return its bounding box
    Box box(vec3(-_r, -_r, -_r), vec3(_r, _r, _r));
    box.transform(_modelToWorld);
    _boundingBox = box;
    _noise = noise;
}

RGB Sphere::getColor(vec3 position) {
    if (_texture == NULL) {
        return _c;
    }
    // Use polar coordinates to compute texture color.
    vec3 point = _worldToModel*position;
    float theta = acos(point[2]/_r);
    float phi = atan2(point[1], point[0]);
    assert(theta >= 0); assert(theta <= M_PI);
    assert(phi >= -M_PI); assert(phi <= M_PI);
    if (phi < 0) phi += 2*M_PI;
    vec2 textureCoord(phi/(2*M_PI), (M_PI-theta)/M_PI);
    vec2 indices(textureCoord[0] * _texture->_width, textureCoord[1] * _texture->_height);
    // Read in the texture image at this coordinate.
    return _texture->_data[int(indices[1])*_texture->_width + int(indices[0])];
}

//Checks for intersection with the given ray
double Sphere::intersect(Ray & ray) {
    Ray ray_t = ray;
    ray_t.transform(_worldToModel);

    // Check intersection of transformed ray with the sphere centered at origin
    float det = pow((ray_t.direction()*(ray_t.start() - _center)), 2) - (ray_t.direction()*ray_t.direction())*(((ray_t.start()-_center)*(ray_t.start()-_center)) - pow(_r,2));
    if (det < 0) return std::numeric_limits<float>::infinity();
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
    if (_noise == NULL && _bumpTexture == NULL) {
        return normal.normalize();
    }
    // We are doing bump mapping.
    float theta = acos(position_t[2]/_r);
    float phi = atan2(position_t[1], position_t[0]);
    assert(theta >= 0); assert(theta <= M_PI);
    assert(phi >= -M_PI); assert(phi <= M_PI);
    if (phi < 0) phi += 2*M_PI;
    vec2 textureCoord(phi/(2*M_PI), (M_PI-theta)/M_PI);
    float x_gradient, y_gradient;
    if (_bumpTexture != NULL) {
        vec2 indices(textureCoord[0] * _bumpTexture->_width, textureCoord[1] * _bumpTexture->_height);
        x_gradient = (_bumpTexture->getValue(indices[0]-1, indices[1]) - _bumpTexture->getValue(indices[0]+1, indices[1]))[RED];
        y_gradient = (_bumpTexture->getValue(indices[0], indices[1]-1) - _bumpTexture->getValue(indices[0], indices[1]+1))[RED];
    } else {
        //_noise is not null.
        x_gradient = _noise->GetHeight(textureCoord[0]-1, textureCoord[1]) - _noise->GetHeight(textureCoord[0]+1, textureCoord[1]);
        y_gradient = _noise->GetHeight(textureCoord[0], textureCoord[1]-1) - _noise->GetHeight(textureCoord[0], textureCoord[1]+1);
    }
    vec3 u(cos(phi), 0, -sin(phi));
    vec3 v(-cos(theta)*sin(phi), sin(theta), -cos(theta)*cos(phi));
    return (normal + u*x_gradient + v*y_gradient).normalize();
}

/****************************************************************
 *													   *
 *		    Triangle                                           *
 *											   		   *
 ****************************************************************/

Triangle::Triangle(vec3 a, vec3 b, vec3 c, RGB & col, Material & m, Texture* t, Texture* bt, mat4 m2w) :Primitive(col,m,t,bt,m2w) {
    verts[0] = a; verts[1] = b; verts[2] = c;
    _verticesHaveNormals = false;
    _verticesHaveTexture = false;
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

Triangle::Triangle(vec3 a, vec3 n1, vec3 b, vec3 n2, vec3 c, vec3 n3, RGB & col, Material & m, Texture* t, Texture* bt, mat4 m2w) : Triangle(a, b, c, col, m, t, bt, m2w) {
    _vertexNormals[0] = n1; _vertexNormals[1] = n2; _vertexNormals[2] = n3;
    _verticesHaveNormals = true;
}

Triangle::Triangle(vec3 a, vec2 t1, vec3 b, vec2 t2, vec3 c, vec2 t3, RGB & col, Material & m, Texture* t, Texture* bt, mat4 m2w) : Triangle(a, b, c, col, m, t, bt, m2w) {
    _textureCoord[0] = t1; _textureCoord[1] = t2; _textureCoord[2] = t3;
    _verticesHaveTexture = (t != NULL);
}

Triangle::Triangle(vec3 a, vec2 t1, vec3 n1, vec3 b, vec2 t2, vec3 n2, vec3 c, vec2 t3, vec3 n3, RGB & col, Material & m, Texture* t, Texture* bt, mat4 m2w) : Triangle(a, n1, b, n2, c, n3, col, m, t, bt, m2w) {
    _textureCoord[0] = t1; _textureCoord[1] = t2; _textureCoord[2] = t3;
    _verticesHaveTexture = (t != NULL);
    _verticesHaveNormals = true;
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
    if (t < 0) return std::numeric_limits<float>::infinity();
    float gamma = C.determinant() / det_A;
    if (gamma < 0 || gamma > 1) return std::numeric_limits<float>::infinity();
    float beta = B.determinant() / det_A;
    if (beta < 0 || beta > 1 - gamma) return std::numeric_limits<float>::infinity();
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
    //assert(alpha >= 0); assert(beta >= 0); assert(gamma >= 0);
    //assert(alpha <= 1); assert(beta <= 1); assert(gamma <= 1);
    if (alpha < 0 || beta < 0 || gamma < 0 || alpha > 1 || beta > 1 || gamma > 1) {
        vec3 normal = _worldToModel.transpose() * vec4(_normal, 0);
        return vec3(normal, VW).normalize();
    }
    vec3 normal = _normal;
    if (alpha >= 0 && alpha <= 1 && beta >=0 && beta <=1 && gamma >=0 && gamma <=1) {
        normal = alpha * _vertexNormals[0] + beta * _vertexNormals[1] + gamma * _vertexNormals[2];
    }
    normal = _worldToModel.transpose() * vec4(normal, 0);
    return vec3(normal, VW).normalize();
}

RGB Triangle::getColor(vec3 position) {
    if (!_verticesHaveTexture) {
        return _c;
    }
    // Use barycentric interpolation to compute texture color.
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
    if (alpha < 0 || beta < 0 || gamma < 0 || alpha > 1 || beta > 1 || gamma > 1) {
        return RGB(0, 0, 0);
    }
    vec2 textureCoord = alpha*_textureCoord[0] + beta*_textureCoord[1] + gamma*_textureCoord[2];
    assert(_texture != NULL);
    vec2 indices(textureCoord[0] * _texture->_width, textureCoord[1] * _texture->_height);
    // Read in the texture image at this coordinate.
    return _texture->_data[int(indices[1])*_texture->_width + int(indices[0])];
}

/****************************************************************
 *													   *
 *		    Cuboid                                             *
 *											   		   *
 ****************************************************************/

Cuboid::Cuboid(vec3 bottomLeft, vec3 topRight, RGB & c, Material & m, mat4 m2w): Primitive(c,m,NULL,NULL,m2w) {
    _bottomLeft = bottomLeft;
    _topRight = topRight;
    Box box(bottomLeft, topRight);
    box.transform(_modelToWorld);
    _boundingBox = box;
}

RGB Cuboid::getColor(vec3 position) {
    return _c;
}

//Checks for intersection with the given ray
double Cuboid::intersect(Ray & ray) {
    Ray ray_t = ray;
    ray_t.transform(_worldToModel);
    vec3 dir = vec3(ray_t.direction(), VW);
    vec3 start = ray_t.start();
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
        return std::numeric_limits<float>::infinity();
    }
    // we have point of interections corresponding to t_x_min, t_y_min or t_z_min depending on which surface has the intersection
    return MAX(t_x_min, MAX(t_y_min, t_z_min));
}

//Calculates the normal for the given position on this sphere.
inline vec3 Cuboid::calculateNormal(vec4 & position) {
    vec3 position_t = _worldToModel*position;
    // Which face does this point lie in ? return normal based on that
    vec3 normal;
    if (DOUBLE_EQ(position_t[0], _bottomLeft[0])) {
        normal = vec3(-1, 0, 0);
    } else if (DOUBLE_EQ(position_t[1], _bottomLeft[1])) {
        normal = vec3(0, -1, 0);
    } else if (DOUBLE_EQ(position_t[2], _bottomLeft[2])) {
        normal = vec3(0, 0, 1);
    } else if (DOUBLE_EQ(position_t[0], _topRight[0])) {
        normal = vec3(1, 0, 0);
    } else if (DOUBLE_EQ(position_t[1], _topRight[1])) {
        normal = vec3(0, 1, 0);
    } else if (DOUBLE_EQ(position_t[2], _topRight[2])) {
        normal = vec3(0, 0, -1);
    } else {
        cout << "Interection point does not lie on cuboid : " << position_t << endl;
        exit(1);
    }
    return vec3(_worldToModel.transpose() * vec4(normal, 0), VW);
}

/****************************************************************
 *													   *
 *		    Cylinder                                           *
 *											   		   *
 ****************************************************************/

Cylinder::Cylinder(double height, double r, RGB & c, Material & m, mat4 m2w): Primitive(c,m,NULL,NULL,m2w) {
    _height = height;
    _r = r;
    Box box(vec3(-r, 0, 0 - r),
            vec3(r, _height, r));
    box.transform(_modelToWorld);
    _boundingBox = box;
}

RGB Cylinder::getColor(vec3 position) {
    return _c;
}

//Checks for intersection with the given ray
double Cylinder::intersect(Ray & ray) {
    Ray ray_t = ray;
    ray_t.transform(_worldToModel);
    vec3 dir = vec3(ray_t.direction(), VW);
    vec3 start = ray_t.start();
    // Intersection with the curved surface.
    double a = pow(dir[0], 2) + pow(dir[2], 2);
    double b = 2 * start[0] * dir[0] + 2 * start[2] * dir[2];
    double c = pow(start[0], 2) + pow(start[2], 2) - 1;
    double det = pow(b, 2) - 4*a*c;
    if (det < 0) {
        return std::numeric_limits<float>::infinity();
    }
    double t1 = (-b - sqrt(det))/(2*a);
    double t2 = (-b + sqrt(det))/(2*a);
    double y1 = vec3(ray_t.getPos(t1))[1];
    double y2 = vec3(ray_t.getPos(t2))[1];
    
    if (y1 >= 0 && y1 <= _height) {
        if (y2 >= 0 && y2 <= _height) {
            if (t1 >= 0 && t2 >= 0) return MIN(y1, y2);
            else if (t1 < 0 && t2 >= 0) return y2;
            else if (t1 >= 0 && t2 < 0) return y1;
        } else {
            if (t1 >= 0) return y1;
        }
    } else if (y2 >= 0 && y2 <= _height) {
        if (t2 >= 0) return y2;
    }
    // Intersection with the capping planes.
    if ((y1 <= 0 && y2 >= 0) || (y1 >= 0 && y2 <= 0)) {
        double t3 = (0 - start[1]) / dir[1];
        if (t3 >= 0) {
            return t3;
        }
    }
    if ((y1 <= _height && y2 >= _height) || (y2 <= _height && y1 >= _height)) {
        double t3 = (_height - start[1]) / dir[1];
        if (t3 >= 0) {
            return t3;
        }
    }
    // No intersection at all.
    return std::numeric_limits<float>::infinity();
}

//Calculates the normal for the given position on this sphere.
inline vec3 Cylinder::calculateNormal(vec4 & position) {
    vec3 position_t = _worldToModel*position;
    // Does this point lie on a face ?
    if (position_t[1] == 0) {
        return vec3(0, -1, 0);
    } else if (position_t[1] == _height) {
        return vec3(0, 1, 0);
    }
    // Must lie on the curved surface
    vec3 normal(position_t[0]/_r, 0, position_t[2]/_r);
    return vec3(_worldToModel.transpose() * vec4(normal, 0), VW);
}

/****************************************************************
 *													   *
 *		    Cone                                               *
 *											   		   *
 ****************************************************************/

Cone::Cone(double height, double r, RGB & c, Material & m, mat4 m2w): Primitive(c,m,NULL,NULL,m2w) {
    _height = height;
    _r = r;
    Box box(vec3(-r, 0, - r),
            vec3(r, _height, r));
    box.transform(_modelToWorld);
    _boundingBox = box;
}

RGB Cone::getColor(vec3 position) {
    return _c;
}

//Checks for intersection with the given ray
double Cone::intersect(Ray & ray) {
    Ray ray_t = ray;
    ray_t.transform(_worldToModel);
    vec3 dir = vec3(ray_t.direction(), VW);
    vec3 start = ray_t.start();
    // Intersection with the curved surface.
    double a = pow(dir[0], 2) + pow(dir[2], 2) - pow(dir[1], 2);
    double b = 2 * start[0] * dir[0] + 2 * start[2] * dir[2] - 2 * start[1] * dir[1];
    double c = pow(start[0], 2) + pow(start[2], 2) - pow(start[1], 2);
    double det = pow(b, 2) - 4*a*c;
    if (det < 0) {
        return std::numeric_limits<float>::infinity();
    }
    double t1 = (-b - sqrt(det))/(2*a);
    double t2 = (-b + sqrt(det))/(2*a);
    double y1 = vec3(ray_t.getPos(t1))[1];
    double y2 = vec3(ray_t.getPos(t2))[1];
    
    if (y1 >= 0 && y1 <= _height) {
        if (y2 >= 0 && y2 <= _height) {
            if (t1 >= 0 && t2 >= 0) return MIN(y1, y2);
            else if (t1 < 0 && t2 >= 0) return y2;
            else if (t1 >= 0 && t2 < 0) return y1;
        } else {
            if (t1 >= 0) return y1;
        }
    } else if (y2 >= 0 && y2 <= _height) {
        if (t2 >= 0) return y2;
    }
    if ((y1 <= _height && y2 >= _height) || (y2 <= _height && y1 >= _height)) {
        return (_height - start[1]) / dir[1];
    }
    // No intersection at all.
    return std::numeric_limits<float>::infinity();
}

//Calculates the normal for the given position on this sphere.
inline vec3 Cone::calculateNormal(vec4 & position) {
    vec3 position_t = _worldToModel*position;
    // Does this point lie on bottom face ?
    if (position_t[1] == _height) {
        return vec3(0, 1, 0);
    }
    double theta = atan(_r/_height);
    double d = position_t[1] / cos(theta);
    double phi = asin(position_t[0]/ (d*sin(theta)));
    // Must lie on the curved surface
    vec3 normal(cos(theta)*sin(phi), -sin(theta), cos(theta)*cos(phi));
    return vec3(_worldToModel.transpose() * vec4(normal, 0), VW);
}

/****************************************************************
 *													   *
 *		    Bounding Box                                       *
 *											   		   *
 ****************************************************************/

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