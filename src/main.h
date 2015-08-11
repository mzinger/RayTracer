/*
 * main.h
 *
 *  Created on: Feb 2, 2009
 *      Author: njoubert
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "global.h"

#include "Viewport.h"
#include "World.h"
#include "Film.h"
#include "Lights.h"
#include <GLUT/glut.h> /* Header File For The GLut Library*/

RGB traceRay(Ray & ray, World* world, int depth);

#endif /* MAIN_H_ */
