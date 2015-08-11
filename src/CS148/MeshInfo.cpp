#include "MeshInfo.h"

#include <sstream>
#include <fstream>

void OBJTriangleMesh::clear() {
    _verticesHaveTexture = false;
    _verticesHaveNormal = false;
    for (vector<OBJTriangle*>::iterator it = triangles.begin(); it != triangles.end(); ++it) {
        delete *it;
    }
    for (vector<OBJVertex*>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
        delete *it;
    }
}

bool getFirstValue(stringstream &ss, VertexTextureNormal &value) {
    string s;
    if (ss >> s) {
        vector<string> returnVal;
        stringstream temp(s);
        string segment;
        while(getline(temp, segment, '/')) {
            returnVal.push_back(segment);
        }
        assert(returnVal.size() >= 1);
        value.vertex = atoi(returnVal[0].c_str()) - 1;
        if (returnVal.size() >= 2) value.texture = atoi(returnVal[1].c_str()) - 1;
        if (returnVal.size() >= 3) value.normal = atoi(returnVal[2].c_str()) - 1;
        return true;
    }
    return false;
}

bool OBJTriangleMesh::loadFile(string file) {
    clear();

    ifstream f(file.c_str());
    if (!f) {
        std::cerr << "Mesh: Couldn't load file " << file << std::endl;
        return false;
    }
    string line;
    // TODO (ankitkr): Investigate why this variable is unused.
    while (getline(f,line)) {
        stringstream linestream(line);
        string op;
        linestream >> op;
        if (op[0] == '#')
            continue;
        if (op == "v") {
            vec3 v;
            linestream >> v;
            vertices.push_back(new OBJVertex(v));
        }
        if (op == "vn") {
            vec3 n;
            linestream >> n;
            normals.push_back(new OBJNormal(n));
        }
        if (op == "vt") {
            vec2 t;
            linestream >> t;
            textures.push_back(new OBJTexture(t));
        }
        if (op == "f") { // extract a face as a triangle fan
            VertexTextureNormal first, second, third;
            if (!getFirstValue(linestream, first))
                continue;
            if (first.normal != -1) _verticesHaveNormal = true;
            if (first.texture != -1) _verticesHaveTexture = true;
            if (!getFirstValue(linestream, second))
                continue;
            while (getFirstValue(linestream, third)) {
                triangles.push_back(new OBJTriangle(first, second, third));
                second = third;
            }
        }
    }
    cout << "Num triangles in mesh : " << triangles.size() << endl;
    return true;
}

bool OBJTriangleMesh::loadGroupFromFile(string file, string groupName) {
    clear();
    
    ifstream f(file.c_str());
    if (!f) {
        std::cerr << "Mesh: Couldn't load file " << file << std::endl;
        return false;
    }
    string line;
    bool reachedGroup = false;
    while (getline(f,line)) {
        stringstream linestream(line);
        string op;
        linestream >> op;
        if (op[0] == '#')
            continue;
        if (op == "v") {
            vec3 v;
            linestream >> v;
            vertices.push_back(new OBJVertex(v));
        }
        if (op == "vn") {
            vec3 n;
            linestream >> n;
            normals.push_back(new OBJNormal(n));
        }
        if (op == "vt") {
            vec2 t;
            linestream >> t;
            textures.push_back(new OBJTexture(t));
        }
        if (op == "g") {
            string name;
            linestream >> name;
            if (reachedGroup) {
                // time to exit
                return true;
            }
            if (name == groupName) {
                reachedGroup = true;
            }
        }
        if (op == "f" && reachedGroup) { // extract a face as a triangle fan
            VertexTextureNormal first, second, third;
            if (!getFirstValue(linestream, first))
                continue;
            if (first.normal != -1) _verticesHaveNormal = true;
            if (first.texture != -1) _verticesHaveTexture = true;
            if (!getFirstValue(linestream, second))
                continue;
            while (getFirstValue(linestream, third)) {
                triangles.push_back(new OBJTriangle(first, second, third));
                second = third;
            }
        }
    }
    return true;
}