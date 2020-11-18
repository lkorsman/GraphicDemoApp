// Draw.cpp - various draw operations, Copyright (c) Jules Bloomenthal, Seattle, 2018, All rights reserved.

#include <glad.h>
#include <gl/glu.h>
#include "Draw.h"
#include "GLXtras.h"
#include <float.h>
#include <stdio.h>

// Screen Mode

void GetViewportSize(int &width, int &height) {
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	width = vp[2];
	height = vp[3];
}

mat4 ScreenMode() {
	int w, h;
	GetViewportSize(w, h);
	mat4 scale = Scale(2.f/(float)w, 2.f/(float)h, 1);
	mat4 tran = Translate(-1, -1, 0);
	return tran*scale;
}

bool IsVisible(vec3 p, mat4 fullview, vec2 *screenA) {
	int width, height;
	GetViewportSize(width, height);
	vec4 xp = fullview*vec4(p, 1);
	vec2 clip(xp.x/xp.w, xp.y/xp.w);	// clip space, +/-1
	vec2 screen(((float) width/2.f)*(1.f+clip.x), ((float) height/2.f)*(1.f+clip.y));
	if (screenA)
		*screenA = screen;
	float z = xp.z/xp.w, zScreen;
	glReadPixels((int)screen.x, (int)screen.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zScreen);
	zScreen = 2*zScreen-1; // seems to work (clip range +/-1 but zbuffer range 0-1)
	return z < zScreen;
}

vec2 ScreenPoint(vec3 p, mat4 m, float *zscreen) {
	int width, height;
	GetViewportSize(width, height);
	vec4 xp = m*vec4(p, 1);
 	if (zscreen)
		*zscreen = xp.z; // /xp.w;
	return vec2(((xp.x/xp.w)+1)*.5f*(float)width, ((xp.y/xp.w)+1)*.5f*(float)height);
}

float ScreenDistSq(int x, int y, vec3 p, mat4 m, float *zscreen) {
	vec2 screen = ScreenPoint(p, m, zscreen);
	float dx = x-screen.x, dy = y-screen.y;
    return dx*dx+dy*dy;
}

void ScreenRay(float xscreen, float yscreen, mat4 modelview, mat4 persp, vec3 &p, vec3 &v) {
	// compute ray from p in direction v; p is transformed eyepoint, xscreen, yscreen determine v
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	// origin of ray is always eye (translated origin)
	p = vec3(modelview[0][3], modelview[1][3], modelview[2][3]);
	// create transposes for gluUnproject
	double tpersp[4][4], tmodelview[4][4], a[3], b[3];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			tmodelview[i][j] = modelview[j][i];
			tpersp[i][j] = persp[j][i];
		}
	// un-project two screen points of differing depth to determine v
	if (gluUnProject(xscreen, yscreen, .25, (const double*) tmodelview, (const double*) tpersp, vp, &a[0], &a[1], &a[2]) == GL_FALSE)
		printf("UnProject false\n");
	if (gluUnProject(xscreen, yscreen, .50, (const double*) tmodelview, (const double*) tpersp, vp, &b[0], &b[1], &b[2]) == GL_FALSE)
		printf("UnProject false\n");
	v = normalize(vec3((float) (b[0]-a[0]), (float) (b[1]-a[1]), (float) (b[2]-a[2])));
}

void ScreenLine(float xscreen, float yscreen, mat4 modelview, mat4 persp, float p1[], float p2[]) {
    // compute 3D world space line, given by p1 and p2, that transforms
    // to a line perpendicular to the screen at (xscreen, yscreen)
	int vp[4];
	double tpersp[4][4], tmodelview[4][4], a[3], b[3];
	// get viewport
	glGetIntegerv(GL_VIEWPORT, vp);
	// create transposes
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			tmodelview[i][j] = modelview[j][i];
			tpersp[i][j] = persp[j][i];
		}
	if (gluUnProject(xscreen, yscreen, .25, (const double*) tmodelview, (const double*) tpersp, vp, &a[0], &a[1], &a[2]) == GL_FALSE)
        printf("UnProject false\n");
	if (gluUnProject(xscreen, yscreen, .50, (const double*) tmodelview, (const double*) tpersp, vp, &b[0], &b[1], &b[2]) == GL_FALSE)
        printf("UnProject false\n");
        // alternatively, a seond point can be determined by transforming the origin by the inverse of modelview
        // this would yield in world space the camera location, through which all view lines pass
	for (int i = 0; i < 3; i++) {
		p1[i] = static_cast<float>(a[i]);
		p2[i] = static_cast<float>(b[i]);
	}
}

// Draw Shader

int drawShader = 0;

const char *drawVShader = "\
	#version 110									\n\
	in vec3 position;								\n\
	in vec3 color;									\n\
	out vec3 vColor;								\n\
    uniform mat4 view;								\n\
	void main() {									\n\
		gl_Position = view*vec4(position, 1);		\n\
		vColor = color;								\n\
	}\n";

const char *drawPShader = "\
	#version 120									\n\
	in vec3 vColor;									\n\
	out vec4 pColor;								\n\
	uniform float opacity = 1;						\n\
	uniform int fadeToCenter = 0;					\n\
	float Fade(float t) {							\n\
		if (t < .95) return 1;						\n\
		if (t > 1.05) return 0;						\n\
		float a = (t-.95)/(1.05-.95);				\n\
		return 1-smoothstep(0, 1, a);				\n\
			// unsure if smoothstep helps			\n\
	}												\n\
	float DistanceToCenter() {						\n\
	    float dx = 1-2*gl_PointCoord.x;				\n\
	    float dy = 1-2*gl_PointCoord.y;				\n\
	    return sqrt(dx*dx+dy*dy);					\n\
	  }												\n\
	void main() {									\n\
		// GL_POINT_SMOOTH deprecated, so calc here	\n\
		// needs GL_POINT_SPRITE enabled			\n\
		float o = opacity;							\n\
	    if (fadeToCenter == 1)						\n\
			o *= Fade(DistanceToCenter());			\n\
	    pColor = vec4(vColor, o);					\n\
	}\n";

void UseDrawShader() {
	if (!drawShader)
		drawShader = LinkProgramViaCode(&drawVShader, &drawPShader);
	glUseProgram(drawShader);
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
//	glEnable(GL_POINT_SMOOTH);
}

void UseDrawShader(mat4 viewMatrix) {
	UseDrawShader();
	SetUniform(drawShader, "view", viewMatrix);
}

// Disks

GLuint diskBuffer = -1;

void Disk(vec3 p, float diameter, vec3 color, float opacity) {
	// diameter should be >= 0, <= 20
	UseDrawShader();
	// create buffer for single vertex (x,y,z,r,g,b)
	if (diskBuffer < 0) {
		glGenBuffers(1, &diskBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, diskBuffer);
		int bufferSize = 6*sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
	}
	glBindBuffer(GL_ARRAY_BUFFER, diskBuffer); // set active buffer
    // allocate buffer memory and load location and color data
	glBufferData(GL_ARRAY_BUFFER, 6*sizeof(float), NULL, GL_STATIC_DRAW); // *** needed??
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3*sizeof(float), &p.x);
	glBufferSubData(GL_ARRAY_BUFFER, 3*sizeof(float), 3*sizeof(float), &color.x);
	// connect shader inputs
	VertexAttribPointer(drawShader, "position", 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
	VertexAttribPointer(drawShader, "color", 3, GL_FLOAT, GL_FALSE, 0, (void *) sizeof(vec3));
	// draw
	SetUniform(drawShader, "fadeToCenter", 1); // needed as GL_POINT_SMOOTH removed
	SetUniform(drawShader, "opacity", opacity);
	glPointSize(diameter);
	glEnable(0x8861); // same as GL_POINT_SPRITE [this is a 4.5 core bug]
	glDrawArrays(GL_POINTS, 0, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Lines

GLuint lineBuffer = -1;

void Line(vec3 p1, vec3 p2, float width, vec3 col, float opacity) {
	UseDrawShader();
 	// create a vertex buffer for the array
	vec3 data[] = {p1, p2, col, col};
	if (lineBuffer <= 0)
		glGenBuffers(1, &lineBuffer);
    // set active vertex buffer, load location and color data
    glBindBuffer(GL_ARRAY_BUFFER, lineBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    // connect shader inputs, set uniforms
	VertexAttribPointer(drawShader, "position", 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    VertexAttribPointer(drawShader, "color", 3, GL_FLOAT, GL_FALSE, 0, (void *) (2*sizeof(vec3))); // sizeof(points));
	SetUniform(drawShader, "fadeToCenter", 0);	// gl_PointCoord fails for lines (instead, use GL_LINE_SMOOTH)
	SetUniform(drawShader, "opacity", opacity);
	// draw
	glLineWidth(width);
	glDrawArrays(GL_LINES, 0, 2);
	// cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Quads

GLuint quadBuffer = -1;

void Quad(vec3 p1, vec3 p2, vec3 p3, vec3 p4, bool solid, vec3 col, float opacity) {
	vec3 data[] = { p1, p2, p3, p4, col, col, col, col };
	UseDrawShader();
	if (quadBuffer < 0) {
		glGenBuffers(1, &quadBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(data), NULL, GL_STATIC_DRAW);
	}
    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	VertexAttribPointer(drawShader, "position", 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    VertexAttribPointer(drawShader, "color", 3, GL_FLOAT, GL_FALSE, 0, (void *) (4*sizeof(vec3)));
	SetUniform(drawShader, "opacity", opacity);
	SetUniform(drawShader, "fadeToCenter", 0);
	glDrawArrays(solid? GL_QUADS : GL_LINE_LOOP, 0, 4);
}

