// Draw.h, Copyright (c) Jules Bloomenthal, 2018, all rights reserved

#ifndef DRAW_HDR
#define DRAW_HDR

#include "VecMat.h"

// screen operations
void GetViewportSize(int &width, int &height);
mat4 ScreenMode();
	// create matrix to map pixel space, (0,0)-(width,height), to NDC (clip) space, (-1,-1)-(1,1)
bool IsVisible(vec3 p, mat4 fullview, vec2 *screen = NULL);
	// if the depth test is enabled, is point p visible?
	// if non-null, set screen location (in pixels) of transformed p
vec2 ScreenPoint(vec3 p, mat4 m, float *zscreen = NULL);
	// transform 3D point to location (xscreen, yscreen), in pixels; if non-null, set zscreen
void ScreenRay(float xscreen, float yscreen, mat4 modelview, mat4 persp, vec3 &p, vec3 &v);
void ScreenLine(float xscreen, float yscreen, mat4 modelview, mat4 persp, float p1[], float p2[]);
    // compute 3D world space line, given by p1 and p2, that transforms
    // to a line perpendicular to the screen at pixel (xscreen, yscreen)
float ScreenDistSq(int x, int y, vec3 p, mat4 m, float *zscreen = NULL);
	// return distance squared, in pixels, between screen point (x, y) and point p xformed by view matrix

// 2D/3D drawing functions
void UseDrawShader();
	// invoke shader for these draw routines
void UseDrawShader(mat4 viewMatrix);
	// as above, but set view transformation
void Disk(vec3 p, float diameter, vec3 color, float opacity = 1);
void Line(vec3 p1, vec3 p2, float width, vec3 col, float opacity = 1);
void Quad(vec3 pnt1, vec3 pnt2, vec3 pnt3, vec3 pnt4, bool solid, vec3 color, float opacity = 1);

#endif
