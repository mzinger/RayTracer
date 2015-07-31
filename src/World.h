/*
 * World.h
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "global.h"
#include "Lights.h"
#include "Primitives.h"

/**
 * The World forms a container for lights and primitives
 * in our scene.
 */
class World {
public:
    World();
    virtual ~World();

    Primitive* intersect(Ray & r, double & t);

    void addPrimitive(Primitive* p);

    void addLight(Light* l);

    void setAmbientLightColor(RGB ambientColor);
    const RGB & getAmbientLightColor();

    vector<Light*>::const_iterator getLightsBeginIterator();
    vector<Light*>::const_iterator getLightsEndIterator();
	
	void printStats();

private:
    vector<Primitive*> _primitives;
    vector<Light*> _lights;
    AmbientLight _ambientLight;
};

#endif /* WORLD_H_ */
