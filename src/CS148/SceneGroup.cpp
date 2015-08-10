/*
 * SceneGroup.cpp
 *
 *  Created on: Feb 12, 2009
 *      Author: jima
 */

#include "SceneInstance.h"
#include "SceneGroup.h"


SceneGroup::SceneGroup()
{
    _name = "unassigned";

    _sphere = NULL;
    _triangle = NULL;
    _cuboid = NULL;
    _cone = NULL;
    _cylinder = NULL;
    _light = NULL;
    _camera = NULL;
    _meshMaterial = NULL;
    _mesh = NULL;
}

// don't delete children, just local stuff; freeing children is SceneLoader's job
// (because cleaning up after a DAG with only local info is not good)
SceneGroup::~SceneGroup() 
{
    delete _sphere;
    delete _light;
    delete _camera;
    delete _mesh;
    delete _cylinder;
    delete _cone;
    delete _cuboid;
    delete _triangle;
}

int SceneGroup::getChildCount()
{
    return int(_children.size());
}

SceneInstance *SceneGroup::getChild(int i) {
    return _children[i];
}

string SceneGroup::getName() {
    return _name;
}

bool SceneGroup::computeMesh(OBJTriangleMesh* &mesh, MaterialInfo &material, int time) { /* get a Mesh and it's material */
    if (_mesh == NULL || _meshMaterial == NULL)
        return false;

    mesh = _mesh;
    material = _meshMaterial->getMaterial(time);

    return true;
}

bool SceneGroup::computeSphere(double &radius, MaterialInfo &material, int time) { /* get a sphere */
    if (_sphere == NULL)
        return false;
    
    radius = _sphere->getRadius(time);
    material = _sphere->getMaterial(time);
    
    return true;
}

bool SceneGroup::computeCylinder(double &radius, double &height, MaterialInfo &material, int time) { /* get a cylinder */
    if (_cylinder == NULL)
        return false;

    radius = _cylinder->getRadius(time);
    height = _cylinder->getHeight(time);
    material = _cylinder->getMaterial(time);
    
    return true;
}

bool SceneGroup::computeCone(double &radius, double &height, MaterialInfo &material, int time) { /* get a cone */
    if (_cone == NULL)
        return false;
    
    radius = _cone->getRadius(time);
    height = _cone->getHeight(time);
    material = _cone->getMaterial(time);
    
    return true;
}

bool SceneGroup::computeTriangle(VertexInfo &vertices, MaterialInfo &material, int time) { /* get a triangle */
    if (_triangle == NULL)
        return false;

    vertices = _triangle->getVertices(time);
    material = _triangle->getMaterial(time);
    
    return true;
}

bool SceneGroup::computeCuboid(VertexInfo &vertices, MaterialInfo &material, int time) { /* get a cuboid */        if (_cuboid == NULL)
        return false;

    vertices = _cuboid->getVertices(time);
    material = _cuboid->getMaterial(time);
    
    return true;
}

bool SceneGroup::computeLight(LightInfo &ld, int time) { /* get light parameters */
    if (_light == NULL)
        return false;
    else
        ld = _light->getLight(time);
    return true;
}

bool SceneGroup::computeCamera(CameraInfo &cam, int time) { /* get camera frustum */
    if (_camera == NULL)
        return false;
    else
        cam = _camera->getCamera(time);
    return true;
}

