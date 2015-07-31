/*
 * Film.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#include "Film.h"

Film::Film(int w, int h, string filename) {
	_w = w;
	_h = h;
	_filename = filename;
}

Film::~Film() {

}

/**
 * Here we stick the "saving" of the image.
 * In our first case, this involves using OpenGL to display the pixels.
 */
void Film::expose(RGB c, Sample & s) {
    	
	//YOUR CODE HERE
	IMPLEMENT_ME(__FILE__,__LINE__);
}

void Film::bakeAndSave() {
	FreeImage_Initialise();	
	FIBITMAP *output = FreeImage_Allocate(_w, _h, 24);
	
	//YOUR CODE HERE
	IMPLEMENT_ME(__FILE__,__LINE__);
	
	FreeImage_Save(FIF_PNG, output, _filename.c_str());
	FreeImage_DeInitialise();
}
