// GLXtras.h - GLSL support, Copyright (c) Jules Bloomenthal, 2018, all rights reserved

#ifndef GL_XTRAS_HDR
#define GL_XTRAS_HDR

#include "VecMat.h"

// Print Info
int PrintGLErrors();
void PrintVersionInfo();
void PrintExtensions();
void PrintProgramLog(int programID);
void PrintProgramAttributes(int programID);
void PrintProgramUniforms(int programID);

// Shader Compilation
int CompileShaderViaFile(const char *filename, GLint type);
int CompileShaderViaCode(const char **code, GLint type);

// Program Linking
int LinkProgramViaCode(const char **vertexCode, const char **pixelCode);
int LinkProgramViaCode(const char **vertexCode, const char **tessellationControlCode, const char **tessellationEvalCode, const char **geometryCode, const char **pixelCode);
int LinkProgram(int vshader, int pshader);
int LinkProgram(int vshader, int tcshader, int teshader, int gshader, int pshader);
int LinkProgramViaFile(const char *vertexShaderFile, const char *pixelShaderFile);

int CurrentShader();

// Uniform Access
bool SetUniform(int shader, const char *name, int val, bool report = true);
bool SetUniformv(int shader, const char *name, int count, int *v, bool report = true);
bool SetUniform(int shader, const char *name, float val, bool report = true);
bool SetUniform(int shader, const char *name, vec2 v, bool report = true);
bool SetUniform(int shader, const char *name, vec3 v, bool report = true);
bool SetUniform(int shader, const char *name, vec3 *v, bool report = true);
bool SetUniform(int shader, const char *name, vec4 *v, bool report = true);
bool SetUniform3(int shader, const char *name, float *v, bool report = true);
bool SetUniform3v(int shader, const char *name, int count, float *v, bool report = true);
bool SetUniform(int shader, const char *name, mat4 m, bool report = true);
	// if no such named uniform and report, print error message

// Attribute Access
int EnableVertexAttribute(int shader, const char *name);
	// find named attribute and enable
void DisableVertexAttribute(int shader, const char *name);
	// find named attribute and disable
void VertexAttribPointer(int shader, const char *name, GLint ncomponents, GLenum datatype, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
	// convenience routine to find and set named attribute

#endif // GL_XTRAS_HDR
