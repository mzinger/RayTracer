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

    FloatRGB* data() {
        return _data;
    }
	
    void BindTexture();
private:
    int _w;
    int _h;
    int _counter;
	string _filename;
    FIBITMAP* _output;
    FloatRGB* _data;
};

#endif /* FILM_H_ */
