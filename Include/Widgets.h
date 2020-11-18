// Widgets.h, copyright (c) Jules Bloomenthal, 2018, all rights reserved.

#ifndef WIDGETS_HDR
#define WIDGETS_HDR

#include <String.h>
#include "VecMat.h"

// mouse
bool MouseOver(int x, int y, vec2 &p, int xoff = 0, int yoff = 0, int proximity = 12);
bool MouseOver(int x, int y, vec3 &p, mat4 &view, int xoff = 0, int yoff = 0, int proximity = 12);
	// is mouse(x,y) within proximity pixels of screen point p?
	// (xoff,yoff) accounts for the displacement between the mouse position and the cursor (xoff,yoff=-7,-3 for GLFW, 3,-12 for GLUT)

// mover
class Mover {
private:
	vec3 *point;
	float plane[4]; // unnormalized
	vec2  mouseOffset;
public:
	Mover() {}
	void Set(vec3 *p, int x, int y, mat4 modelview, mat4 persp);
	void Drag(int x, int y, mat4 modelview, mat4 persp);
//	void Drag(int x, int y, mat4 &modelview, mat4 &persp);
	bool Hit(int x, int y, mat4 &view, int xoff = 0, int yoff = 0, int proximity = 12);
	bool IsSet();
	void Unset();
};

// toggle button
class Toggler {
private:
	bool *on;
	std::string name;
	int x, y;
	float dia;
	vec3 onCol, offCol, ringCol;
public:
	Toggler(bool *on, const char *name, int x, int y, float dia = 12, vec3 onCol = vec3(1,0,0), vec3 offCol = vec3(.7f), vec3 ringCol = vec3(.1f));
	bool On();
	void Draw();
	bool Hit(int xMouse, int yMouse, int xoff = 0, int yoff = 0, int proximity = 12);
	bool UpHit(int xMouse, int yMouse, int state, int xoff = 0, int yoff = 0, int proximity = 12);
	const char *Name();
};

#endif
