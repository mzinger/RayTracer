/*
 * World.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#include "World.h"

World::World() {
}

World::~World() {
    // TODO Auto-generated destructor stub
}

Primitive* World::intersect(Ray & r, double & t) {

    IMPLEMENT_ME(__FILE__,__LINE__);

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

void World::printStats() {
	cout << "World data:" << endl;
	cout << " primitives: " << _primitives.size() << endl;
	cout << " lights: " << _lights.size() << endl;
	
}
