// RotatingColorfulLetter.cpp: rotates a colorful letter made of triangles

#include <glad.h>
#include <glfw3.h>
#include <stdio.h>
#include "GLXtras.h"
#include <time.h>

time_t startTime = clock();		// Start rotation timer
static float degPerSec = 30;	// Degrees letter will rotate

// GPU identifiers
GLuint vBuffer = 0;
GLuint program = 0;

// vertices (3 2D locations, 3 RGB colors);
float  colors[][3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
					   {1, 1, .5}, {.2, .2, .2} , {.8, 0, 1},
					   {.125, 1, .4}, {1, .2, .2}, {1, 1, 1},
						{0.0, 0.0, 0.0} };
float  points[][2] = { {-.6f, -.8f}, {-.2f, -.8f}, {.4f, -.8f},
						{.8f, -.8f}, {-.2f, 0.0f}, {.2, 0.0f},
						{-.6f, .8f}, {-.2f, .8f}, {.4f, .8f},
						{.8f, .8f} };
// Triangles
int triangles[][3] = { {0, 1, 4},
						{4, 5, 2},
						{2, 3, 5},
						{5, 8, 9},
						{4, 5, 8},
						{4, 6, 7},
						{0, 4, 6} };

const char *vertexShader = "\
	#version 120								\n\
	in vec2 point;								\n\
	in vec3 color;								\n\
	out vec4 vColor;							\n\
	uniform float angle = 0;					\n\
	vec2 Rotate(vec2 p) {						\n\
		float rad = 3.1415926 * angle / 180.0;	\n\
		float c = cos(rad);						\n\
		float s = sin(rad);						\n\
		float x2 = c * p.x - s * p.y;			\n\
		float y2 = p.y * c + p.x * s;			\n\
		return vec2(x2, y2);					\n\
	}											\n\
	void main() {								\n\
		gl_Position = vec4(Rotate(point), 0, 1);\n\
	    vColor = vec4(color, 1);				\n\
	}";

const char *pixelShader = "\
	#version 120								\n\
	in vec4 vColor;								\n\
	out vec4 pColor;							\n\
	void main() {								\n\
        pColor = vColor;						\n\
	}";

void InitVertexBuffer() {
    // make GPU buffer for points & colors, set it active buffer
    glGenBuffers(1, &vBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    // allocate buffer memory to hold vertex locations and colors
	int sPnts = sizeof(points), sCols = sizeof(colors);
    glBufferData(GL_ARRAY_BUFFER, sPnts+sCols, NULL, GL_STATIC_DRAW);
    // load data to the GPU
    glBufferSubData(GL_ARRAY_BUFFER, 0, sPnts, points);
        // start at beginning of buffer, for length of points array
    glBufferSubData(GL_ARRAY_BUFFER, sPnts, sCols, colors);
        // start at end of points array, for length of colors array
}

void Display() {
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT);
	// access GPU vertex buffer
    glUseProgram(program);
	// Update the angle
	float elapsedTime = (float)(clock() - startTime) / CLOCKS_PER_SEC;
	float angle = elapsedTime * degPerSec;
	SetUniform(program, "angle", angle);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    // associate position input to shader with position array in vertex buffer
	VertexAttribPointer(program, "point", 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    // associate color input to shader with color array in vertex buffer
	VertexAttribPointer(program, "color", 3, GL_FLOAT, GL_FALSE, 0, (void *) sizeof(points));
	// render eighteen vertices as six triangles
	glDrawElements(GL_TRIANGLES, 21, GL_UNSIGNED_INT, triangles);
    glFlush();
}

// application

void ErrorGFLW(int id, const char *reason) {
    printf("GFLW error %i: %s\n", id, reason);
}

void Keyboard(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main() {
    glfwSetErrorCallback(ErrorGFLW);
    if (!glfwInit())
        return 1;
    GLFWwindow *window = glfwCreateWindow(600, 600, "Letter from Triangles", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    printf("GL version: %s\n", glGetString(GL_VERSION));
    PrintGLErrors();
	if (!(program = LinkProgramViaCode(&vertexShader, &pixelShader)))
        return 0;
    InitVertexBuffer();
    glfwSetKeyCallback(window, Keyboard);
	while (!glfwWindowShouldClose(window)) {
		Display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
    glfwDestroyWindow(window);
    glfwTerminate();
}
