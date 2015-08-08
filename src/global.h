/*
 * global.h
 *
 *  Created on: Feb 2, 2009
 *      Author: njoubert
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#define IMPLEMENT_ME(file,line)    cout << "METHOD NEEDS TO BE IMPLEMENTED AT " << file << ":" << line << endl; exit(1);

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <vector>

#include <cmath>
#include <cassert>
#include <cstdlib>

//Include our math library
#include <algebra3.h>

#include <GL/freeglut.h>

#define IMAGE_WIDTH 512
#define IMAGE_HEIGHT 512
#define RAYS_PER_PIXEL 1

#define OPENGL_RENDER (1)
#define USE_ACCELERATION_INDEX (1)
#define USE_VERTEX_NORMALS (1)
#define USE_REFRACTION (0)
#define MAX_TIME (100)
#define USE_DEPTH_OF_FIELD (0)

#endif /* GLOBAL_H_ */
