// Text.h: text support

#ifndef TEXT_HDR
#define TEXT_HDR

#include <glad.h>
#include <GLFW/glfw3.h>
#include "GLXtras.h"

class Character {
public:
    GLuint	textureID;	// glyph texture
    int2	gSize;		// glyph size
    int2	bearing;    // offset from baseline to left/top of glyph
    GLuint	advance;	// offset to next glyph
	Character() { }
    Character(int textureID, int2 gSize, int2 bearing, GLuint advance) :
		textureID(textureID), gSize(gSize), bearing(bearing), advance(advance) { }
};

// character set and current pointer
struct CharacterSet {
	Character characters[128];
	CharacterSet() { }
	CharacterSet(const CharacterSet &cs) { for (int i = 0; i < 128; i++) characters[i] = cs.characters[i]; }
};

CharacterSet *SetFont(const char *fontName, int charRese = 15, int pixelRes = 15);

void Text(int x, int y, vec3 color, float scale, const char *format, ...);
	// position null-terminated text at pixel (x, y)

void Text(vec3 p, mat4 m, vec3 color, float scale, const char *format, ...);
	// position text on screen per point p transformed by m

void RenderText(const char *text, float x, float y, vec3 color, float scale, mat4 view);
	// text with arbitrary orientation

#endif
