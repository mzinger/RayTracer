#include "MeshInfo.h"

#include <sstream>
#include <fstream>

void OBJTriangleMesh::clear() {
    for (vector<OBJTriangle*>::iterator it = triangles.begin(); it != triangles.end(); ++it) {
        delete *it;
    }
    for (vector<OBJVertex*>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
        delete *it;
    }
}

bool getFirstValue(stringstream &ss, int &value) {
    string s;
    if (ss >> s) {
        stringstream valuestream(s);
        if (valuestream >> value)
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
    int normals = 0;
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
        if (op == "f") { // extract a face as a triangle fan
            int first, second, third;
            if (!getFirstValue(linestream, first))
                continue;
            if (!getFirstValue(linestream, second))
                continue;
            while (getFirstValue(linestream, third)) {
                triangles.push_back(
                                    new OBJTriangle(first-1, second-1, third-1)
                                   );
                second = third;
            }
        }
    }
    return true;
}
