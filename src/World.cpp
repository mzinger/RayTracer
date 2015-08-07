/*
 * World.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */
#include <limits.h>
#include <unordered_map>

#include "World.h"

World::World() {
    _bvhTree = NULL;
}

World::~World() {
    // TODO Auto-generated destructor stub
}

Primitive* World::intersect(Ray & r, double & t) {
    t = std::numeric_limits<double>::max();
    Primitive* result = nullptr;
    if (_bvhTree == NULL) {
        // Linear lookup over primitives for intersection.
        for (auto* primitive : _primitives) {
            double curr_t = primitive->intersect(r);
            if (curr_t < t && curr_t > r.getMinT()) {
                t = curr_t;
                result = primitive;
            }
        }
    } else {
        // Traverse the BVHTree index to compute intersections efficiently
        result = _bvhTree->intersect(r, t);
    }
    return result;
}

void World::addPrimitive(Primitive* p) {
    _primitives.push_back(p);
}

void World::addLight(Light* l) {
    _lights.push_back(l);
}

void World::setAmbientLightColor(RGB ambientColor) {
    _ambientLight.setColor(ambientColor);
}

const RGB & World::getAmbientLightColor() {
    return _ambientLight.getColor();
}

vector<Light*>::const_iterator World::getLightsBeginIterator() {
    return _lights.begin();
}

vector<Light*>::const_iterator World::getLightsEndIterator() {
    return _lights.end();
}

void World::PreprocessToBHVTree() {
    _bvhTree = new BVHNode(_primitives, 0);  // start splittling from X axes.
}

struct vec3Hash {
    size_t operator()(const vec3& k) const {
        return ((hash<double>()(k[0]) ^ (hash<double>()(k[1]) << 1)) >> 1) ^ (hash<double>()(k[2]) << 1);
    }
};

void World::PreprocessTriangleNormals() {
    // Go through the triangles of the scene, find faces sharing a vertex, assign vertex normal to their mean.
    unordered_map<vec3, vector<Triangle*>, vec3Hash> triangle_map;
    for (Primitive* primitive : _primitives) {
        if (Triangle* tri = dynamic_cast<Triangle*>(primitive)) {
            triangle_map[tri->verts[0]].push_back(tri);
            triangle_map[tri->verts[1]].push_back(tri);
            triangle_map[tri->verts[2]].push_back(tri);
        }
    }
    for (auto& it : triangle_map) {
        vec3 normal(0, 0, 0);
        for (Triangle* tri : it.second) {
            normal += tri->_normal;
        }
        normal.normalize();
        // Reflect back to the triangles.
        for (Triangle* tri : it.second) {
            if (tri->verts[0] == it.first) {
                tri->_vertexNormals[0] = normal;
            } else if (tri->verts[1] == it.first) {
                tri->_vertexNormals[1] = normal;
            } else if (tri->verts[2] == it.first) {
                tri->_vertexNormals[2] = normal;
            } else {
                cout << "Vertex not in triangle ? This should never happen." << endl;
                exit(1);
            }
            tri->_verticesHaveNormals = true;
        }
    }
}

void World::printStats() {
	cout << "World data:" << endl;
	cout << " primitives: " << _primitives.size() << endl;
	cout << " lights: " << _lights.size() << endl;
	
}
