/*
 * Film.h
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 */

#ifndef FILM_H_
#define FILM_H_

#include "global.h"

#include <FreeImage.h>

class Film {
public:
    Film(int w, int h, string filename);
    virtual ~Film();

    void expose(RGB c, Sample & s);
	
	void bakeAndSave();
	
private:
    int _w;
    int _h;
	string _filename;
    
};

#endif /* FILM_H_ */
