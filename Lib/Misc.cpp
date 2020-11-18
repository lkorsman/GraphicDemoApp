// Misc.cpp, Copyright (c) Jules Bloomenthal, Seattle, 2018, All rights reserved.

#include <glad.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include "Misc.h"

// Geometry

float RaySphere(vec3 base, vec3 v, vec3 center, float radius) {
	// return least pos alpha of ray and sphere (or -1 if none)
	vec3 q = base-center;
	float vDot = dot(v, q);
	float sq = vDot*vDot-dot(q, q)+radius*radius;
	if (sq < 0)
		return -1;
	float root = sqrt(sq), a = -vDot-root;
	return a > 0? a : -vDot+root;
}

// Image File IO

char *ReadTarga(std::string filename, int &width, int &height) {
	// open targa file, read header, return pointer to pixels
	FILE *in = fopen(filename.c_str(), "rb");
	if (in) {
		short tgaHeader[9];
		fread(tgaHeader, sizeof(tgaHeader), 1, in);
		// allocate, read pixels
		int w = width = tgaHeader[6], h = height = tgaHeader[7];
		int bitsPerPixel = tgaHeader[8], bytesPerPixel = bitsPerPixel/8, bytesPerImage = w*h*bytesPerPixel;
		if (bytesPerPixel != 3) {
			printf("bytes per pixel not 3!\n");
			return NULL;
		}
		char *pixels = new char[bytesPerImage];
		fread(pixels, bytesPerImage, 1, in);
		return pixels;
	}
	printf("can't open %s\n", filename.c_str());
	return NULL;
}

// Texture

void SetTexture(const char *filename, GLuint textureUnit, GLuint textureName) {
	// open targa file, read header, store as mipmap texture
	int width, height;
	char *pixels = ReadTarga(filename, width, height);
	// set and bind texture
	glActiveTexture(GL_TEXTURE1+textureUnit);	// active texture corresponds with textureUnit
	glBindTexture(GL_TEXTURE_2D, textureName);	// bind active texture to textureName
	// allocate GPU texture buffer; copy, free pixels
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);		// accommodate width not multiple of 4
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	delete [] pixels;
}

// Color

vec3 RGBfromHSV(vec3 hsv) {
	hsv.x *= 360.f;                       // text expects h to be [0..360] but we use [0..1]
	hsv.x = fmod(hsv.x, 360.f);           // degrees cannot exceed 360
	if (abs(hsv.y) < FLT_MIN)             // color is on the black-and-white center line
		return vec3(hsv.z, hsv.z, hsv.z);
	if (abs(hsv.x-360.f) < FLT_MIN)       // change 360 to 0 degrees
		hsv.x = 0;
	hsv.x = hsv.x/60.f;                   // h now in [0,6]
	int i = (int) floor(hsv.x);           // floor returns the largest integer <= h
	float f = hsv.x-i;
	float p = hsv.z*(1-hsv.y);
	float q = hsv.z*(1-(hsv.y*f));
	float t = hsv.z*(1-(hsv.y*(1-f)));
	switch (i) {
		case 0:  return vec3(hsv.z, t, p);
		case 1:  return vec3(q, hsv.z, p);
		case 2:  return vec3(p, hsv.z, t);
		case 3:  return vec3(p, q, hsv.z);
		case 4:  return vec3(t, p, hsv.z);
		default: return vec3(hsv.z, p, q);
	}
}

