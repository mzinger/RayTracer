#include "SceneLoader.h"

#include "algebra3.h"

#include <fstream>
#include <string>
#include <sstream>

using namespace std;


SceneLoader::SceneLoader(Scene &scene, string file) {
    err = &cout;
    scene._root = new SceneInstance();
    root = scene._root;
    root->_name = "toplevel";
    buildScene(file);
}

SceneLoader::~SceneLoader() {
    for (map<string, SceneGroup*>::iterator it = groups.begin(); 
                                                 it != groups.end(); ++it)
    {
        delete it->second;
    }
    for (map<string, ParametricMaterial*>::iterator it = materials.begin(); 
                                                 it != materials.end(); ++it)
    {
        delete it->second;
    }
    for (vector<SceneInstance*>::iterator it = instances.begin(); it != instances.end(); ++it)
    {
        delete *it;
    }
}

void SceneLoader::buildEndlineTable(string filename)
{
    ifstream file(filename.c_str());
    string line;
    while (getline( file, line ))
        endlineTable.push_back(file.tellg());
    endlineTable.push_back(file.tellg());
}

void SceneLoader::curPos(ostream &out, int g)
{
    int line = 1;
    int lastg = 0;
    for (vector<int>::iterator it = endlineTable.begin(); it != endlineTable.end(); ++it)
    {
        if (*it > g) {
            out << "line: " << line << " char: " << (g - lastg - 1);
            return;
        }

        lastg = *it;
        line ++;
    }
    out << "line: " << line << " char: " << (g - lastg);
}

string SceneLoader::getString(istream &str)
{
    string ret;
    char temp;
    str >> temp; // get rid of white space, get to first char
    str.putback(temp);
    while (str.get(temp))
    {
        if ((temp >= 'a' && temp <= 'z')
                || (temp >= 'A' && temp <= 'Z')
                || (temp >= '0' && temp <= '9')
                || (temp == '_') )
        {
            ret += temp;
        }
        else if (temp == '#')
        {
            string no;
            getline(str, no);
            if (ret.empty())
                return getString(str);
            else
                return ret;
        }
        else {
            str.putback(temp);
            break;
        }
    }

    return ret;
}

string SceneLoader::getQuoted(istream &str)
{
    string ret;
    char temp;
    str >> temp;
    if (temp != '"') {
        *err << "expected opening \" at ";
        curPos(*err, str.tellg());
        *err << endl;
    }
    getline(str, ret, '"');
    return ret;
}

bool SceneLoader::readCommand(istream &str, string &name)
{
    name = getString(str);

    if (name.empty()) {
        *err << "error: expected command but did not find one at ";
        curPos(*err, str.tellg());
        *err << endl;
        return false;
    }

    return true;
}

bool SceneLoader::findOpenParen(istream &str)
{
    char temp;
    string line;
    while (str >> temp)
    {
        if (temp == '(')
            return true;
        else if (temp == '#')
            getline(str,line);
        else {
            *err << "error: unexpected char " << temp << " at ";
            curPos(*err, str.tellg());
            *err << endl;
            return false;
        }
    }
    return false;
}

enum { OPEN, CLOSED, ERROR };  // possible types of paren, used by the findOpenOrClosedParen function below
int SceneLoader::findOpenOrClosedParen(istream &str)
{
    char temp;
    string line;
    while (str >> temp)
    {
        if (temp == '(')
            return OPEN;
        else if (temp == '#')
            getline(str,line);
        else if (temp == ')') {
            str.putback(temp);
            return CLOSED;
        }
        else {
            *err << "error: unexpected char " << temp << " at ";
            curPos(*err, str.tellg());
            *err << endl;
            return false;
        }
    }
    return ERROR;
}

bool SceneLoader::findCloseParen(istream &str)
{
    char temp;
    string line;
    int close = 0;
    while (str >> temp)
    {
        if (temp == '#')
            getline(str, line);
        if (temp == '(')
            close++;
        if (temp == ')')
            close--;
        if (close < 0)
            return true;
    }
    return false;
}

int SceneLoader::getValues(istream &str, vector<ParametricValue*> &vals)
{
    ParametricValue *val;
    while ( (val = getValue(str)) != NULL )
    {
        vals.push_back(val);
    }
    return int(vals.size());
}

ParametricValue* SceneLoader::getValue(istream &str)
{
    char temp;
    str >> temp;
    if (temp == '{') // time-varying expression
    {
        int start = str.tellg();
        string expr;
        while (str.get(temp))
        {
            if (temp == '}') // completed expr
            {
                ParametricValue *v = new ExprValue(expr.c_str());
                if (!v->good()) {
                    *err << "Error: couldn't parse expression \"" << expr << "\" at ";
                    curPos(*err,str.tellg());
                    *err << endl;
                    delete v;
                    return NULL;
                } else {
                    return v;
                }
            }
            else if (temp == '#')
            {
                string no;
                getline(str, no);
            }
            else {
                expr += temp;
            }
        }
        *err << "Error: No closing brace for expr at ";
        curPos(*err, start);
        *err << endl;
        return NULL;
    }
    else if (temp == ')')
    {
        str.putback(temp);
        return NULL;
    }
    else if (temp == '#')
    {
        string no;
        getline(str,no);
        return getValue(str);
    }
    else
    {
        str.putback(temp);

        double val;
        str.peek();
        str >> val;
        if (str.fail())
        {
            str.clear();
            *err << "Failed to extract a numeric value at ";
            curPos(*err, str.tellg());
            *err << endl;

            str.peek();
            return NULL;
        } else {
            return new ConstValue(val);
        }
    }
}

void SceneLoader::errLine(int at)
{
    curPos(*err, at);
    *err << endl;
}

void SceneLoader::cleanAfter(vector<ParametricValue*> &vals, unsigned int i)
{
    for (;i<vals.size(); i++)
    {
        delete vals[i];
    }
}

SceneInstance* SceneLoader::doI(istream &str, string &name)
{
    name = getString(str);
    if (name.empty()) {
        *err << "Couldn't read instance name at "; errLine(str.tellg());
        return NULL;
    }

    string var = getString(str);
    if (groups[var] == NULL)
    {
        *err << "Instancing node " << var << " which doesn't exist yet at ";
        curPos(*err,str.tellg());
        *err << endl;
        return NULL;
    }

    SceneInstance *n = new SceneInstance();
    instances.push_back(n); //nodes[name] = n;
    n->_name = name;
    n->_child = groups[var];

    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR)
            return NULL;
        else if (state == CLOSED)
            return n;
        else if (state == OPEN)
        {
            string cmd;
            vector<ParametricValue*> values;
            if (readCommand(str, cmd)) {

                if (cmd == "R") // load rotations
                {
                    int numv = getValues(str, values);
                    Rotate *r = NULL;
                    if (numv < 1) {
                        *err << "R with no args at "; errLine(str.tellg());
                    } else if (numv < 4) {
                        cleanAfter(values, 1);
                        r = new Rotate();
                        r->angle = values[0];
                    } else {
                        cleanAfter(values, 4);
                        r = new Rotate();
                        r->angle = values[0];
                        for (int i = 0; i < 3; i++)
                            r->axis[i] = values[i+1];
                    }
                    if (r != NULL)
                        n->_transforms.push_back(r);
                }
                else if (cmd == "Xform")
                {
                    int numv = getValues(str, values);
                    GeneralTransform *g = NULL;
                    if (numv < 9) {
                        *err << "Xform with too few parameters at "; errLine(str.tellg());
                    } else if (numv < 16) { // 2d
                        cleanAfter(values, 9);
                        g = new GeneralTransform();
                        for (int i = 0; i < 9; i++)
                            g->matrix.push_back(values[i]);
                    } else {
                        cleanAfter(values, 16);
                        g = new GeneralTransform();
                        for (int i = 0; i < 16; i++)
                            g->matrix.push_back(values[i]);
                    }
                    if (g != NULL)
                        n->_transforms.push_back(g);
                }
                else if (cmd == "T")
                {
                    int numv = getValues(str, values);
                    Translate *t = NULL;
                    if (numv < 2) {
                        *err << "T with too few parameters at "; errLine(str.tellg());
                    } else if (numv == 2) {
                        t = new Translate();
                        for (int i = 0; i < 2; i++)
                            t->translate[i] = values[i];
                    } else {
                        cleanAfter(values, 3);
                        t = new Translate();
                        for (int i = 0; i < 3; i++)
                            t->translate[i] = values[i];
                    }
                    if (t != NULL)
                        n->_transforms.push_back(t);
                }
                else if (cmd == "S")
                {
                    int numv = getValues(str, values);
                    Scale *s = NULL;
                    if (numv < 2) {
                        *err << "S with too few parameters at "; errLine(str.tellg());
                    } else if (numv == 2) {
                        s = new Scale();
                        for (int i = 0; i < 2; i++)
                            s->scale[i] = values[i];
                    } else {
                        s = new Scale();
                        for (int i = 0; i < 3; i++)
                            s->scale[i] = values[i];
                    }
                    if (s != NULL)
                        n->_transforms.push_back(s);
                }
                else if (cmd == "color")
                {
                    int numv = getValues(str, values);
                    ParametricColor *c = NULL;
                    if (numv < 3) {
                        *err << "color with too few parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 3);
                        c = new ParametricColor();
                        for (int i = 0; i < 3; i++)
                            c->_color[i] = values[i];
                    }
                    if (c != NULL)
                        n->_color = c;
                }
                else if (cmd == "lod")
                {
                    int numv = getValues(str, values);
                    LOD *l = NULL;
                    if (numv < 1) {
                        *err << "lod with no parameters at "; errLine(str.tellg());
                    } else {
                        //cout << "got lod" << endl;
                        cleanAfter(values, 1);
                        l = new LOD();
                        l->_level = values[0];
                    }
                    if (l != NULL)
                        n->_lod = l;
                }
                else
                {
                    *err << "Error: command " << cmd << " not recognized at ";
                    curPos(*err,str.tellg());
                    *err << endl;
                }
                findCloseParen(str);
            }
        }
    } while (true);
}

bool SceneLoader::getName(istream &str, string type, string &name)
{
    name = getString(str);
    if (name.empty()) {
        *err << "Couldn't read " << type << " name at "; errLine(str.tellg());
        return false;
    }

    if (groups[name] != NULL)
    {
        *err << "Illegal re-use of name \"" << name << "\" at "; errLine(str.tellg());
        return false;
    }

    return true;
}

void SceneLoader::SetMaterialDefaults(ParametricMaterial *n)
{
    if (n->_RGB == NULL) {
        n->_RGB = new ParametricColor();
        n->_RGB->_color[0] = new ConstValue(0);
        n->_RGB->_color[1] = new ConstValue(0);
        n->_RGB->_color[2] = new ConstValue(0);
    }
    for (int i = 0; i < 7; i++)
    {
        if (n->_coefficients[i] == NULL)
            n->_coefficients[i] = new ConstValue(0);
    }
}

bool SceneLoader::doMaterial(istream &str, string &name)
{
    name = getString(str);
    if (name.empty()) {
        *err << "Couldn't read material name at "; errLine(str.tellg());
        return false;
    }

    if (materials[name] != NULL) {
        *err << "Illegal re-use of name \"" << name << "\" at "; errLine(str.tellg());
        return false;
    }

    map<string, int> indices;
    indices["ma"] = MAT_MA;
    indices["ml"] = MAT_MD;
    indices["ms"] = MAT_MS;
    indices["msp"] = MAT_MSP;
    indices["msm"] = MAT_MSM;
    indices["mt"] = MAT_MT;
    indices["mtn"] = MAT_MTN;


    ParametricMaterial *n = new ParametricMaterial();
    materials[name] = n;
    
    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR) {
            SetMaterialDefaults(n);
            return false;
        } else if (state == CLOSED) {
            SetMaterialDefaults(n);
            return true;
        }
        else if (state == OPEN)
        {
            string cmd;
            vector<ParametricValue*> values;
            if (readCommand(str, cmd)) {
                if (cmd == "color") {
                    if (getValues(str, values) < 3) {
                        *err << "Color with insufficent parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 3);
                        n->_RGB = new ParametricColor();
                        for (int ci = 0; ci < 3; ci++)
                            n->_RGB->_color[ci] = values[ci];
                    }
                } else if (cmd == "texture") {
                    string texture_filename = getQuoted(str);
                    cout << "Reading texture from file : " << texture_filename << endl;
                    n->_texture = new Texture(texture_filename);
                } else if (cmd == "PerlinBumpMap") {
                    if (getValues(str, values) < 5) {
                        *err << cmd << " with insufficient parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 5);
                        n->_noise = new PerlinNoise(values[0]->getValue(0), values[1]->getValue(0), values[2]->getValue(0), values[3]->getValue(0), values[4]->getValue(0));
                    }
                } else if (cmd == "TextureBumpMap") {
                    string texture_filename = getQuoted(str);
                    cout << "Reading texture bump map from file : " << texture_filename << endl;
                    n->_bumpTexture = new Texture(texture_filename);
                } else if (indices.find(cmd) != indices.end()) {
                    if (getValues(str, values) < 1) {
                        *err << cmd << " with no parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_coefficients[indices[cmd]] = values[0];
                    }
                } else {
                    *err << "Error: command " << cmd << " not recognized at "; errLine(str.tellg());
                }
                findCloseParen(str);
            }
        }
    } while (true);
}

void SceneLoader::SetSphereDefaults(SceneGroup *n)
{
    cout << "! set sphere defaults" << endl;
    if (n->_sphere->_material == NULL)
    {
        n->_sphere->_material = new ParametricMaterial();
        SetMaterialDefaults(n->_sphere->_material);
    }
    if (n->_sphere->_radius == NULL)
    {
        n->_sphere->_radius = new ConstValue(1);
    }
}

// no obj includes in as5; they come back in as6
bool SceneLoader::doInclude(istream &str, string& name)
{
    if (!getName(str, "include", name))
        return false;

    SceneGroup *n = new SceneGroup();
    groups[name] = n;
    n->_name = name;

    string file = getQuoted(str);
    n->_mesh = new OBJTriangleMesh(file);
    
    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR) {
            if (n->_meshMaterial == NULL) {
                n->_meshMaterial = new ParametricMaterial();
                SetMaterialDefaults(n->_meshMaterial);
            }
            return false;
        } else if (state == CLOSED) {
            if (n->_meshMaterial == NULL) {
                n->_meshMaterial = new ParametricMaterial();
                SetMaterialDefaults(n->_meshMaterial);
            }
            return true;
        } else if (state == OPEN) {
            string cmd;
            if (readCommand(str, cmd)) {
                if (cmd == "material") {
                    string matName = getString(str);
                    if (matName.empty()) {
                        *err << "No material name after material command at "; errLine(str.tellg());
                    } else if (materials[matName] == NULL) {
                        *err << "Unknown material " << matName << " referenced at "; errLine(str.tellg());
                    } else {
                        n->_meshMaterial = materials[matName];
                    }
                } else {
                    *err << "Error: command " << cmd << " not recognized at "; errLine(str.tellg());
                }
                findCloseParen(str);
            }
        }
    } while (true);
}

void SceneLoader::getOBJMeshes(string& file, vector<OBJMeshInfo>& meshes) {
    ifstream f(file.c_str());
    if (!f) {
        std::cerr << "MatMesh: Couldn't load file " << file << std::endl;
        return;
    }
    string line, groupName, matName;
    while (getline(f,line)) {
        stringstream linestream(line);
        string op;
        linestream >> op;
        if (op == "g") {
            string name;
            linestream >> groupName;
            string line2;
            getline(f,line2);
            stringstream linestream2(line2);
            linestream2 >> op;
            if (op == "usemtl") {
                string name;
                linestream2 >> matName;
            }
            OBJMeshInfo newMesh;
            newMesh.groupName = groupName;
            newMesh.matName = matName;
            meshes.push_back(newMesh);
            cout << "Found new group in MatMesh : " << groupName << "@" << matName << endl;
        }
    }
}

bool SceneLoader::doMatMesh(istream &str)
{
    string file = getQuoted(str);
    vector<OBJMeshInfo> meshes;
    getOBJMeshes(file, meshes);
    
    for (OBJMeshInfo mesh : meshes) {
        SceneGroup *n = new SceneGroup();
        groups[mesh.groupName] = n;
        n->_name = mesh.groupName;
        n->_mesh = new OBJTriangleMesh(file, mesh.groupName);
        n->_meshMaterial = materials[mesh.matName];
    }
    return true;
}

bool SceneLoader::doMtllib(istream &str) {
    string file = getQuoted(str);
    ifstream f(file.c_str());
    if (!f) {
        std::cerr << "Mtllib: Couldn't load file " << file << std::endl;
        return false;
    }
    string line;
    ParametricMaterial *curr = NULL;
    while (getline(f,line)) {
        stringstream linestream(line);
        string op, name;
        linestream >> op;
        if (op == "newmtl") {
            linestream >> name;
            curr = new ParametricMaterial();
            materials[name] = curr;
        } else if (op == "d") {
            // unsupported
            continue;
        } else if (op == "Ns") {
            double a;
            linestream >> a;
            if (curr != NULL) {
                ParametricValue* val = new ConstValue(a);
                curr->_coefficients[MAT_MSP] = val;
            } else {
                cout << "Mtllib : Attempting to set property of undefined material" << endl;
            }
        } else if (op == "Ni") {
            //unsupported
            continue;
        } else if (op == "Ka") {
            vec3 amb;
            linestream >> amb;
            if (curr != NULL) {
                ParametricValue* val = new ConstValue(amb[0]);
                curr->_coefficients[MAT_MA] = val;
            } else {
                cout << "Mtllib : Attempting to set property of undefined material" << endl;
            }
        } else if (op == "Kd") {
            vec3 lamb;
            linestream >> lamb;
            if (curr != NULL) {
                curr->_RGB = new ParametricColor();
                for (int i = 0; i < 3; ++i) {
                    ParametricValue* val = new ConstValue(lamb[i]);
                    curr->_RGB->_color[i] = val;
                }
                ParametricValue* val = new ConstValue(0.6);
                curr->_coefficients[MAT_MD] = val;
            } else {
                cout << "Mtllib : Attempting to set property of undefined material" << endl;
            }
        } else if (op == "Ks") {
            vec3 spec;
            linestream >> spec;
            if (curr != NULL) {
                ParametricValue* val = new ConstValue(spec[0]);
                curr->_coefficients[MAT_MS] = val;
            } else {
                cout << "Mtllib : Attempting to set property of undefined material" << endl;
            }
        } else if (op == "Km") {
            // unsuported
            continue;
        } else if (op == "map_Kd") {
            string texture_filename;
            linestream >> texture_filename;
            cout << "Mtllib : Loading texture from file : " << texture_filename << endl;
            curr->_texture = new Texture(texture_filename);
        } else if (op == "map_Bump") {
            // not supported
            continue;
        }
    }
    return true;
}

bool SceneLoader::doSphere(istream &str, string &name)
{
    if (!getName(str, "sphere", name))
        return false;

    SceneGroup *n = new SceneGroup();
    groups[name] = n;
    n->_name = name;

    n->_sphere = new ParametricSphere();

    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR) {
            SetSphereDefaults(n);
            return false;
        } else if (state == CLOSED) {
            SetSphereDefaults(n);
            return true;
        } else if (state == OPEN)
        {
            string cmd;
            vector<ParametricValue*> values;
            if (readCommand(str, cmd)) {
                if (cmd == "radius") {
                    if (getValues(str, values) < 1) {
                        *err << "Type with no parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_sphere->_radius = values[0];
                    }
                } else if (cmd == "material") {
                    string matName = getString(str);
                    if (matName.empty()) {
                        *err << "No material name after material command at "; errLine(str.tellg());
                    } else if (materials[matName] == NULL) {
                        *err << "Unknown material " << matName << " referenced at "; errLine(str.tellg());
                    } else {
                        n->_sphere->_material = materials[matName];
                    }
                } else {
                    *err << "Error: command " << cmd << " not recognized at "; errLine(str.tellg());
                }
                findCloseParen(str);
            }
        }
    } while (true);
}

void SceneLoader::SetTriangleDefaults(SceneGroup *n)
{
    // set default values for the vertices
    if (n->_triangle->_vertices[0] == NULL)
        n->_triangle->_vertices[0] = new ConstValue(0);
    if (n->_triangle->_vertices[1] == NULL)
        n->_triangle->_vertices[1] = new ConstValue(0);
    if (n->_triangle->_vertices[2] == NULL)
        n->_triangle->_vertices[2] = new ConstValue(0);
    if (n->_triangle->_vertices[3] == NULL)
        n->_triangle->_vertices[3] = new ConstValue(0);
    if (n->_triangle->_vertices[4] == NULL)
        n->_triangle->_vertices[4] = new ConstValue(0);
    if (n->_triangle->_vertices[5] == NULL)
        n->_triangle->_vertices[5] = new ConstValue(0);
}

bool SceneLoader::doTriangle(istream &str, string &name)
{
    if (!getName(str, "triangle", name))
        return false;
    
    SceneGroup *n = new SceneGroup();
    groups[name] = n;
    n->_name = name;
    
    n->_triangle = new ParametricTriangle();
    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR) {
            SetTriangleDefaults(n);
            return false;
        } else if (state == CLOSED) {
            SetTriangleDefaults(n);
            return true;
        } else if (state == OPEN)
        {
            string cmd;
            vector<ParametricValue*> values;
            if (readCommand(str, cmd)) {
                if (cmd == "vertices") {
                    if (getValues(str, values) < 6) {
                        *err << "vertices with insufficient parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 6);
                        n->_triangle->_vertices[0] = values[0];
                        n->_triangle->_vertices[1] = values[1];
                        n->_triangle->_vertices[2] = values[2];
                        n->_triangle->_vertices[3] = values[3];
                        n->_triangle->_vertices[4] = values[4];
                        n->_triangle->_vertices[5] = values[5];
                    }
                } else if (cmd == "material") {
                    string matName = getString(str);
                    if (matName.empty()) {
                        *err << "No material name after material command at "; errLine(str.tellg());
                    } else if (materials[matName] == NULL) {
                        *err << "Unknown material " << matName << " referenced at "; errLine(str.tellg());
                    } else {
                        n->_triangle->_material = materials[matName];
                    }
                } else {
                    *err << "Error: command " << cmd << " not recognized at "; errLine(str.tellg());
                }
                findCloseParen(str);
            }
        }
    } while (true);
}

void SceneLoader::SetCuboidDefaults(SceneGroup *n)
{
    // set default values for the vertices
    if (n->_cuboid->_vertices[0] == NULL)
        n->_cuboid->_vertices[0] = new ConstValue(0);
    if (n->_cuboid->_vertices[1] == NULL)
        n->_cuboid->_vertices[1] = new ConstValue(0);
    if (n->_cuboid->_vertices[2] == NULL)
        n->_cuboid->_vertices[2] = new ConstValue(0);
    if (n->_cuboid->_vertices[3] == NULL)
        n->_cuboid->_vertices[3] = new ConstValue(0);
    if (n->_cuboid->_vertices[4] == NULL)
        n->_cuboid->_vertices[4] = new ConstValue(0);
    if (n->_cuboid->_vertices[5] == NULL)
        n->_cuboid->_vertices[5] = new ConstValue(0);
    if (n->_cuboid->_material == NULL)
    {
        n->_cuboid->_material = new ParametricMaterial();
        SetMaterialDefaults(n->_cuboid->_material);
    }
}

bool SceneLoader::doCuboid(istream &str, string &name)
{
    if (!getName(str, "cuboid", name))
        return false;
    
    SceneGroup *n = new SceneGroup();
    groups[name] = n;
    n->_name = name;
    
    n->_cuboid = new ParametricCuboid();
    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR) {
            SetCuboidDefaults(n);
            return false;
        } else if (state == CLOSED) {
            SetCuboidDefaults(n);
            return true;
        } else if (state == OPEN)
        {
            string cmd;
            vector<ParametricValue*> values;
            if (readCommand(str, cmd)) {
                if (cmd == "vertices") {
                    if (getValues(str, values) < 6) {
                        *err << "vertices with insufficient parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 6);
                        n->_cuboid->_vertices[0] = values[0];
                        n->_cuboid->_vertices[1] = values[1];
                        n->_cuboid->_vertices[2] = values[2];
                        n->_cuboid->_vertices[3] = values[3];
                        n->_cuboid->_vertices[4] = values[4];
                        n->_cuboid->_vertices[5] = values[5];
                    }
                } else if (cmd == "material") {
                    string matName = getString(str);
                    if (matName.empty()) {
                        *err << "No material name after material command at "; errLine(str.tellg());
                    } else if (materials[matName] == NULL) {
                        *err << "Unknown material " << matName << " referenced at "; errLine(str.tellg());
                    } else {
                        n->_cuboid->_material = materials[matName];
                    }
                } else {
                    *err << "Error: command " << cmd << " not recognized at "; errLine(str.tellg());
                }
                findCloseParen(str);
            }
        }
    } while (true);
}

void SceneLoader::SetCylinderDefaults(SceneGroup *n)
{
    // set default values
    if (n->_cylinder->_radius == NULL)
        n->_cylinder->_radius = new ConstValue(1);
    if (n->_cylinder->_height == NULL)
        n->_cylinder->_height = new ConstValue(1);
    if (n->_cylinder->_material == NULL)
    {
        n->_cylinder->_material = new ParametricMaterial();
        SetMaterialDefaults(n->_cylinder->_material);
    }
}

bool SceneLoader::doCylinder(istream &str, string &name)
{
    if (!getName(str, "cylinder", name))
        return false;
    
    SceneGroup *n = new SceneGroup();
    groups[name] = n;
    n->_name = name;
    
    n->_cylinder = new ParametricCylinder();
    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR) {
            SetCylinderDefaults(n);
            return false;
        } else if (state == CLOSED) {
            SetCylinderDefaults(n);
            return true;
        } else if (state == OPEN)
        {
            string cmd;
            vector<ParametricValue*> values;
            if (readCommand(str, cmd)) {
                if (cmd == "radius") {
                    if (getValues(str, values) < 1) {
                        *err << "radius with insufficient parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_cylinder->_radius = values[0];
                    }
                } else if (cmd == "height") {
                    if (getValues(str, values) < 1) {
                        *err << "height with insufficient parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_cylinder->_height = values[0];
                    }
                } else if (cmd == "material") {
                    string matName = getString(str);
                    if (matName.empty()) {
                        *err << "No material name after material command at "; errLine(str.tellg());
                    } else if (materials[matName] == NULL) {
                        *err << "Unknown material " << matName << " referenced at "; errLine(str.tellg());
                    } else {
                        n->_cylinder->_material = materials[matName];
                    }
                } else {
                    *err << "Error: command " << cmd << " not recognized at "; errLine(str.tellg());
                }
                findCloseParen(str);
            }
        }
    } while (true);
}

void SceneLoader::SetConeDefaults(SceneGroup *n)
{
    // set default values
    if (n->_cone->_radius == NULL)
        n->_cone->_radius = new ConstValue(1);
    if (n->_cone->_height == NULL)
        n->_cone->_height = new ConstValue(1);
    if (n->_cone->_material == NULL)
    {
        n->_cone->_material = new ParametricMaterial();
        SetMaterialDefaults(n->_cone->_material);
    }

}

bool SceneLoader::doCone(istream &str, string &name)
{
    if (!getName(str, "cone", name))
        return false;
    
    SceneGroup *n = new SceneGroup();
    groups[name] = n;
    n->_name = name;
    
    n->_cone = new ParametricCone();
    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR) {
            SetConeDefaults(n);
            return false;
        } else if (state == CLOSED) {
            SetConeDefaults(n);
            return true;
        } else if (state == OPEN)
        {
            string cmd;
            vector<ParametricValue*> values;
            if (readCommand(str, cmd)) {
                if (cmd == "radius") {
                    if (getValues(str, values) < 1) {
                        *err << "radius with insufficient parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_cone->_radius = values[0];
                    }
                } else if (cmd == "height") {
                    if (getValues(str, values) < 1) {
                        *err << "height with insufficient parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_cone->_height = values[0];
                    }
                } else if (cmd == "material") {
                    string matName = getString(str);
                    if (matName.empty()) {
                        *err << "No material name after material command at "; errLine(str.tellg());
                    } else if (materials[matName] == NULL) {
                        *err << "Unknown material " << matName << " referenced at "; errLine(str.tellg());
                    } else {
                        n->_cone->_material = materials[matName];
                    }
                } else {
                    *err << "Error: command " << cmd << " not recognized at "; errLine(str.tellg());
                }
                findCloseParen(str);
            }
        }
    } while (true);
}

void SceneLoader::SetLightDefaults(SceneGroup *n) {
    // set default values for unset variables
    if (n->_light->_angularFalloff == NULL)
        n->_light->_angularFalloff = new ConstValue(0);
    if (n->_light->_color == NULL) {
        n->_light->_color = new ParametricColor();
        n->_light->_color->_color[0] = new ConstValue(0);
        n->_light->_color->_color[1] = new ConstValue(0);
        n->_light->_color->_color[2] = new ConstValue(0);
    }
    if (n->_light->_deadDistance == NULL)
        n->_light->_deadDistance = new ConstValue(.1);
    if (n->_light->_falloff == NULL)
        n->_light->_falloff = new ConstValue(0);
    if (n->_light->_type == NULL)
        n->_light->_type = new ConstValue(LIGHT_AMBIENT);
}

bool SceneLoader::doLight(istream &str, string &name)
{
    if (!getName(str, "light", name))
        return false;

    SceneGroup *n = new SceneGroup();
    groups[name] = n;
    n->_name = name;

    n->_light = new ParametricLight();

    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR) {
            SetLightDefaults(n);
            return false;
        } else if (state == CLOSED) {
            SetLightDefaults(n);
            return true;
        }
        else if (state == OPEN)
        {
            string cmd;
            vector<ParametricValue*> values;
            if (readCommand(str, cmd)) {
                if (cmd == "type") {
                    if (getValues(str, values) < 1) {
                        *err << "Type with no parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_light->_type = values[0];
                    }
                } else if (cmd == "falloff") {
                    if (getValues(str, values) < 1) {
                        *err << "Falloff with no parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_light->_falloff = values[0];
                    }
                } else if (cmd == "deaddistance") {
                    if (getValues(str, values) < 1) {
                        *err << "deaddistance with no parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_light->_deadDistance = values[0];
                    }
                } else if (cmd == "angularfalloff") {
                    if (getValues(str, values) < 1) {
                        *err << "angularfalloff with no parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_light->_angularFalloff = values[0];
                    }
                } else if (cmd == "color") {
                    if (getValues(str, values) < 3) {
                        *err << "color with insufficient parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 3);
                        n->_light->_color = new ParametricColor();
                        n->_light->_color->_color[0] = values[0];
                        n->_light->_color->_color[1] = values[1];
                        n->_light->_color->_color[2] = values[2];
                    }
                } else {
                    *err << "Error: command " << cmd << " not recognized at "; errLine(str.tellg());
                }
                findCloseParen(str);
            }
        }
    } while (true);
}

void SceneLoader::SetCameraDefaults(SceneGroup *n)
{
    if (n->_camera->_perspective == NULL)
        n->_camera->_perspective = new ConstValue(1); // default perspective to 1
    
    // set default values for the frustum
    if (n->_camera->_frustum[0] == NULL)
        n->_camera->_frustum[0] = new ConstValue(-.33);
    if (n->_camera->_frustum[1] == NULL)
        n->_camera->_frustum[1] = new ConstValue(+.33);
    if (n->_camera->_frustum[2] == NULL)
        n->_camera->_frustum[2] = new ConstValue(-.33);
    if (n->_camera->_frustum[3] == NULL)
        n->_camera->_frustum[3] = new ConstValue(+.33);
    if (n->_camera->_frustum[4] == NULL)
        n->_camera->_frustum[4] = new ConstValue(-1);
    if (n->_camera->_frustum[5] == NULL)
        n->_camera->_frustum[5] = new ConstValue(-100);
}

bool SceneLoader::doCamera(istream &str, string &name)
{
    if (!getName(str, "camera", name))
        return false;

    SceneGroup *n = new SceneGroup();
    groups[name] = n;
    n->_name = name;

    n->_camera = new ParametricCamera();
    

    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR) {
            SetCameraDefaults(n);
            return false;
        } else if (state == CLOSED) {
            SetCameraDefaults(n);
            return true;
        }
        else if (state == OPEN)
        {
            string cmd;
            vector<ParametricValue*> values;
            string sides = "lrbtnf";
            int side = 0;
            if (readCommand(str, cmd)) {
                if (cmd == "perspective") {
                    if (getValues(str, values) < 1) {
                        *err << "Perspective with no parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_camera->_perspective = values[0];
                    }
                }
                else if (cmd.size() == 1 && (side = (int)sides.find(cmd)) != string::npos) {
                    if (getValues(str, values) < 1) {
                        *err << "l with no parameters at "; errLine(str.tellg());
                    } else {
                        cleanAfter(values, 1);
                        n->_camera->_frustum[side] = values[0];
                    }
                }
                else
                {
                    *err << "Error: command " << cmd << " not recognized at "; errLine(str.tellg());
                }
                findCloseParen(str);
            }
        }
    } while (true);
}

bool SceneLoader::doG(istream &str, string &name)
{
    name = getString(str);
    if (name.empty()) {
        *err << "Couldn't read group name at "; errLine(str.tellg());
        return false;
    }

    if (groups[name] != NULL)
    {
        *err << "Illegal re-use of group name \"" << name << "\" at ";
        curPos(*err,str.tellg());
        *err << endl;
        return false;
    }

    SceneGroup *n = new SceneGroup();
    groups[name] = n;
    n->_name = name;

    do {
        int state = findOpenOrClosedParen(str);
        if (state == ERROR)
            return false;
        else if (state == CLOSED)
            return true;
        else if (state == OPEN)
        {
            string cmd;
            if (readCommand(str, cmd)) {
                if (cmd != "I")
                {
                    *err << "Command other than I from G at ";
                    curPos(*err,str.tellg());
                    *err << endl;
                }
                string iname;
                SceneInstance *newNode;
                if ((newNode = doI(str,iname)) != NULL) {
                    n->_children.push_back( newNode );
                }
                findCloseParen(str);
            }
        }
    } while (true);
}


bool SceneLoader::doRender(istream &str, string &name)
{
    name = getString(str);
    if (name.empty())
    {
        *err << "Trying to render group without specifying a name at ";
        curPos(*err,str.tellg());
        *err << endl;
        return false;
    }

    if (groups[name] == NULL)
    {
        *err << "Trying to render group not found \"" << name << "\" at ";
        curPos(*err,str.tellg());
        *err << endl;
        return false;
    }

    root->_child = groups[name];

    return true;
}

bool SceneLoader::buildScene(string filename)
{
    buildEndlineTable(filename);

    ifstream file(filename.c_str());
    string line;
    int lastPos = 0;
    while (findOpenParen(file))
    {
        file.tellg();
        if (readCommand(file, line)) {
            if (line == "Include")
            {
                string instName;
                if (doInclude(file, instName))
                {
                    cout << "included " << instName << endl;
                }
                else
                {
                    cout << "mangled include at ";
                    curPos(cout, file.tellg());
                    cout << endl;
                }
            } else if (line == "MatMesh") {
                if (doMatMesh(file))
                {
                    cout << "included MatMesh" << endl;
                }
                else
                {
                    cout << "mangled include MatMesh at ";
                    curPos(cout, file.tellg());
                    cout << endl;
                }
            } else if (line == "Mtllib") {
                if (doMtllib(file))
                {
                    cout << "included mtllib " << endl;
                }
                else
                {
                    cout << "mangled include mtllib at ";
                    curPos(cout, file.tellg());
                    cout << endl;
                }
            } else if (line == "Sphere")
            {
                string gname;
                if (doSphere(file, gname))
                {
                    cout << "read sphere " << gname << endl;
                }
                else
                {
                    *err << "mangled sphere command at "; errLine(file.tellg());
                }
            }
            else if (line == "Triangle")
            {
                string gname;
                if (doTriangle(file, gname))
                {
                    cout << "read Triangle " << gname << endl;
                }
                else
                {
                    *err << "mangled triangle command at "; errLine(file.tellg());
                }
            }
            else if (line == "Cuboid")
            {
                string gname;
                if (doCuboid(file, gname))
                {
                    cout << "read Cuboid " << gname << endl;
                }
                else
                {
                    *err << "mangled cuboid command at "; errLine(file.tellg());
                }
            }
            else if (line == "Cylinder")
            {
                string gname;
                if (doCylinder(file, gname))
                {
                    cout << "read Cylinder " << gname << endl;
                }
                else
                {
                    *err << "mangled cylinder command at "; errLine(file.tellg());
                }
            }
            else if (line == "Cone")
            {
                string gname;
                if (doCone(file, gname))
                {
                    cout << "read Cone " << gname << endl;
                }
                else
                {
                    *err << "mangled cone command at "; errLine(file.tellg());
                }
            }
            else if (line == "Material")
            {
                string gname;
                if (doMaterial(file, gname))
                {
                    cout << "read material " << gname << endl;
                }
                else
                {
                    *err << "mangled material command at "; errLine(file.tellg());
                }
            }
            else if (line == "Light")
            {
                string gname;
                if (doLight(file, gname))
                {
                    cout << "read light " << gname << endl;
                }
                else
                {
                    *err << "mangled light command at "; errLine(file.tellg());
                }
            }
            else if (line == "Camera")
            {
                string gname;
                if (doCamera(file, gname))
                {
                    cout << "read camera " << gname << endl;
                }
                else
                {
                    *err << "mangled camera command at "; errLine(file.tellg());
                }
            }
            else if (line == "I")
            {
                *err << "Error: Instance commands must belong to a group, but I found in global scope at "; errLine(file.tellg());
                /*string iname; // code to handle I at global scope (doesn't make much sense now that instance names skip the names table)
                if (doI(file, iname))
                {
                    cout << "got an instance named " << iname << endl;
                }*/
            }
            else if (line == "G")
            {
                string iname;
                if (doG(file, iname))
                {
                    cout << "got a group named " << iname << endl;
                }
            }
            else if (line == "Render")
            {
                string iname;
                if (doRender(file, iname))
                {
                    cout << "did render " << iname << endl;
                }
            }
            else
            {
                *err << "command not recognized: " << line << endl;
            }
            findCloseParen(file);
        } else {

        }

        lastPos = file.tellg();
    }

    return true;
}
