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

#define IMAGE_WIDTH 1024
#define IMAGE_HEIGHT 800
#define RAYS_PER_PIXEL 1
#define RAYS_PER_PIXEL_SAMPLE 3 // relevant if depth of field is on
#define CAMERA_LENS_SIZE 0.1 // relevant if depth of field is on
#define RAYS_PER_LIGHT_SOURCE 3 // relevant if soft-shadows are enabled

#define OPENGL_RENDER (1)
#define MAX_BOUNCES (3)
#define USE_ACCELERATION_INDEX (1)
#define USE_VERTEX_NORMALS (1)
#define USE_REFRACTION (0)
#define MAX_TIME (1)
#define DEPTH_OF_FIELD (0)
#define POINT_LIGHT_SOURCE_SIZE 0 // Soft-shadows; set to 0 to disable

#endif /* GLOBAL_H_ */
