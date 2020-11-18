// Misc.h, Copyright (c) Jules Bloomenthal, 2018, all rights reserved

#ifndef MISC_HDR
#define MISC_HDR

#include "VecMat.h"

// geometry
float RaySphere(vec3 base, vec3 v, vec3 center, float radius);
	// return least pos alpha of ray and sphere (or -1 if none)

// image file
char *ReadTarga(std::string filename, int &width, int &height);
	// allocate width*height pixels, set them from file and return pointer
	// this memory should be freed by the caller
	// expects 24 bpp

// texture
void SetTexture(const char *filename, GLuint textureUnit, GLuint textureName);

// color
vec3 RGBfromHSV(vec3 hsv);

#endif
