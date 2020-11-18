// Widgets.cpp, Copyright (c) Jules Bloomenthal, Seattle, 2018, All rights reserved.

#include <glad.h>
#include <glfw3.h>
#include <gl/glu.h>
#include "Draw.h"
#include "GLXtras.h"
#include "Misc.h"
#include "Widgets.h"
#include <stdio.h>
#include <string.h>

// #define USE_TEXT

#ifdef USE_TEXT
	#include "Text.h"
#endif

// Mouse

bool MouseOver(int x, int y, vec2 &p, int xoff, int yoff, int proximity) {
	vec2 m(x+xoff, y+yoff);
	return length(m-p) < proximity;
}

bool MouseOver(int x, int y, vec3 &p, mat4 &view, int xoff, int yoff, int proximity) {
	return ScreenDistSq(x+xoff, y+yoff, p, view) < proximity*proximity;
}

// Mover

float DotProduct(float a[], float b[]) { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }

/* deprecated
void SetPlane(vec3 &p, int x, int y, mat4 &modelview, mat4 &persp, float *plane) {
	vec3 p1, p2;										// two points that transform to pixel x,y
	ScreenLine((float) x, (float) y, modelview, persp, (float *) &p1, (float *) &p2);
	for (int i = 0; i < 3; i++)
		plane[i] = p2[i]-p1[i];							// set plane normal (not unitized) to that of line p1p2
	plane[3] = -plane[0]*p.x-plane[1]*p.y-plane[2]*p.z;	// pass plane through point
} */

void Mover::Set(vec3 *p, int x, int y, mat4 modelview, mat4 persp) {
	vec2 s = ScreenPoint(*p, persp*modelview);
	mouseOffset = vec2(s.x-x, s.y-y);
	point = p;
//	SetPlane(*p, x, y, modelview, persp, plane);
	for (int i = 0; i < 3; i++)
		plane[i] = modelview[2][i];  // zrow (which is product of modelview matrix with untransformed z-axis)
	plane[3] = -plane[0]*p->x-plane[1]*p->y-plane[2]*p->z;	// pass plane through point
}

void Mover::Drag(int xMouse, int yMouse, mat4 modelview, mat4 persp) {
	if (!point)
		return;
	float p1[3], p2[3], axis[3];
	float x = xMouse+mouseOffset.x, y = yMouse+mouseOffset.y;
	ScreenLine(static_cast<float>(x), static_cast<float>(y), modelview, persp, p1, p2);
	// get two points that transform to pixel x,y
	for (int i = 0; i < 3; i++)
		axis[i] = p2[i]-p1[i];
	// direction of line through p1
	float pdDot = DotProduct(axis, plane);
	// project onto plane normal
	float a = (-plane[3]-DotProduct(p1, plane))/pdDot;
	// intersection of line with plane
	for (int j = 0; j < 3; j++)
		(*point)[j] = p1[j]+a*axis[j];
}

bool Mover::Hit(int x, int y, mat4 &view, int xoff, int yoff, int proximity) {
	return MouseOver(x, y, *point, view, xoff, yoff, proximity);
}

bool Mover::IsSet() {
	return point != NULL;
}

void Mover::Unset() {
	point = NULL;
}

// Toggler

Toggler::Toggler(bool *on, const char *name, int x, int y, float dia, vec3 onCol, vec3 offCol, vec3 ringCol)
	: on(on), name(name), x(x), y(y), dia(dia), onCol(onCol), offCol(offCol), ringCol(ringCol) { };

void Toggler::Draw() {
	// assume ScreenMode and no depth-test
	vec3 p((float)x, (float)y, 0);
	Disk(p, dia, ringCol);
	Disk(p, dia-6, *on? onCol : offCol);
#ifdef USE_TEXT
	Text(x+15, y-5, vec3(0,0,0), 1, name.c_str());
#endif
}

bool Toggler::Hit(int xMouse, int yMouse, int xoff, int yoff, int proximity) {
	vec2 p((float)x, (float)y);
	return MouseOver(xMouse, yMouse, p, xoff, yoff, proximity);
}

bool Toggler::UpHit(int xMouse, int yMouse, int state, int xoff, int yoff, int proximity) {
	bool hit = Hit(xMouse, yMouse, xoff, yoff, proximity);
	if (state == GLFW_RELEASE && hit)
		*on = *on? false : true;
	return hit;
}

bool Toggler::On() {
	return *on;
}

const char *Toggler::Name() {
	return name.c_str();
}
