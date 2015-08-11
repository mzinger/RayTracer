/*
 * MeshInfo.h
 *
 * Basic mesh data
 *
 *  Created on: Feb 19, 2009
 *      Author: jima
 */

#ifndef MESH_INFO_H_
#define MESH_INFO_H_

#include "algebra3.h"
#include <vector>

struct VertexTextureNormal {
public:
    int vertex;
    int texture;
    int normal;
    VertexTextureNormal() : vertex(-1), texture(-1), normal(-1) {};
    VertexTextureNormal(int v, int t, int n) : vertex(v), texture(t), normal(n) {};
};

struct OBJVertex {
    vec3 pos;

    OBJVertex(vec3 pos) : pos(pos) {}
};

struct OBJNormal {
    vec3 dir;
    OBJNormal(vec3 dir) : dir(dir) {}
};

struct OBJTexture {
    vec2 coords;
    OBJTexture(vec2 coords) : coords(coords) {}
};

struct OBJTriangle {
    int ind[3];
    int normal_ind[3];
    int texture_ind[3];

    // position, texture then normal.
    OBJTriangle(VertexTextureNormal& i, VertexTextureNormal& j, VertexTextureNormal& k) {
        ind[0] = i.vertex; ind[1] = j.vertex; ind[2] = k.vertex;
        texture_ind[0] = i.texture; texture_ind[1] = j.texture; texture_ind[2] = k.texture;
        normal_ind[0] = i.normal; normal_ind[1] = j.normal; normal_ind[2] = k.normal;
    }
};

struct OBJTriangleMesh {
    std::vector<OBJTriangle*> triangles;
    std::vector<OBJVertex*> vertices;
    std::vector<OBJNormal*> normals;
    std::vector<OBJTexture*> textures;

    OBJTriangleMesh() : _verticesHaveNormal(false), _verticesHaveTexture(false) {}
    OBJTriangleMesh(string file) : _verticesHaveNormal(false), _verticesHaveTexture(false) {
        loadFile(file);
    }
    OBJTriangleMesh(string file, string groupName) : _verticesHaveNormal(false), _verticesHaveTexture(false) {
        loadGroupFromFile(file, groupName);
    }
    ~OBJTriangleMesh() { 
        clear(); 
    }

    bool verticesHaveNormal() { return _verticesHaveNormal;}
    bool verticesHaveTexture() { return _verticesHaveTexture;}
    bool loadFile(string file);
    bool loadGroupFromFile(string file, string groupName);
    void clear();
private:
    bool _verticesHaveNormal;
    bool _verticesHaveTexture;
};

#endif // MESH_INFO_H_