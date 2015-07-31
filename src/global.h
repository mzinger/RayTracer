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

//Include our math library
#include <algebra3.h>

#define IMAGE_WIDTH 512
#define IMAGE_HEIGHT 512
#define RAYS_PER_PIXEL 4


#endif /* GLOBAL_H_ */
