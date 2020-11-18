// GLXtras.cpp - GLSL support, Copyright (c) Jules Bloomenthal, 2011, 2012, All rights reserved

#include <glad.h>
#include <gl/glu.h>
#include "GLXtras.h"
#include <stdio.h>
#include <string.h>

// Support

int PrintGLErrors() {
    char buf[1000];
    int nErrors = 0;
    buf[0] = 0;
    for (;;) {
        GLenum n = glGetError();
        if (n == GL_NO_ERROR)
            break;
        sprintf(buf+strlen(buf), "%s%s", !nErrors++? "" : ", ", gluErrorString(n));
            // do not call Debug() while looping through errors, so accumulate in buf
    }
    if (nErrors)
		printf("%s\n", nErrors? buf : "no GL errors");
    return nErrors;
}

// Print OpenGL, GLSL Details

void PrintVersionInfo() {
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *version     = glGetString(GL_VERSION);
    const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("GL vendor: %s\n", vendor);
    printf("GL renderer: %s\n", renderer);
    printf("GL version: %s\n", version);
    printf("GLSL version: %s\n", glslVersion);
 // GLint major, minor;
 // glGetIntegerv(GL_MAJOR_VERSION, &major);
 // glGetIntegerv(GL_MINOR_VERSION, &minor);
 // printf("GL version (integer): %d.%d\n", major, minor);
}

void PrintExtensions() {
    const GLubyte *extensions = glGetString(GL_EXTENSIONS);
	const char *skip = "(, \t\n";
	char buf[100];
    printf("\nGL extensions:\n");
		if (extensions)
			for (char *c = (char *) extensions; *c; ) {
					c += strspn(c, skip);
					size_t nchars = strcspn(c, skip);
					strncpy(buf, c, nchars);
					buf[nchars] = 0;
					printf("  %s\n", buf);
					c += nchars;
	}
}

void PrintProgramLog(int programID) {
    GLint logLen;
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLen);
    if (logLen > 0) {
        char *log = new char[logLen];
        GLsizei written;
        glGetProgramInfoLog(programID, logLen, &written, log);
        printf("Program log:\n%s", log);
        delete [] log;
    }
}

void PrintProgramAttributes(int programID) {
    GLint maxLength, nAttribs;
    glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTES, &nAttribs);
    glGetProgramiv(programID, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
    char *name = new char[maxLength];
    GLint written, size;
    GLenum type;
    for (int i = 0; i < nAttribs; i++) {
        glGetActiveAttrib(programID, i, maxLength, &written, &size, &type, name);
        GLint location = glGetAttribLocation(programID, name);
        printf("    %-5i  |  %s\n", location, name);
    }
    delete [] name;
}

void PrintProgramUniforms(int programID) {
    GLenum type;
    GLchar name[201];
    GLint nUniforms, length, size;
    glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &nUniforms);
    printf("  uniforms\n");
    for (int i = 0; i < nUniforms; i++) {
        glGetActiveUniform(programID, i,  200,  &length, &size, &type, name);
        printf("    %s\n", name);
    }
}

// Compilation

int CompileShaderViaFile(const char *filename, GLint type) {
		FILE* fp = fopen(filename, "r");
		if (fp == NULL)
			return 0;
		fseek(fp, 0L, SEEK_END);
		long maxSize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		char *buf = new char[maxSize+1], c;
		int nchars = 0;
		while ((c = fgetc(fp)) != EOF)
			buf[nchars++] = c;
		buf[nchars] = 0;
		fclose(fp);
		return CompileShaderViaCode((const char **) &buf, type);
}

int CompileShaderViaCode(const char **code, GLint type) {
    GLuint shader = glCreateShader(type);
	if (!shader) {
		PrintGLErrors();
		return false;
	}
    glShaderSource(shader, 1, code, NULL);
	glCompileShader(shader);
    // check compile status
    GLint result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        // report logged errors
        GLint logLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 0) {
            GLsizei written;
            char *log = new char[logLen];
            glGetShaderInfoLog(shader, logLen, &written, log);
            printf(log);
            delete [] log;
        }
        else
            printf("shader compilation failed\n");
        return 0;
    }
    // const char *t = type == GL_VERTEX_SHADER? "vertex" : "pixel";
    // printf("  %s shader (%i) successfully compiled\n", t, shader);
    return shader;
}

// Linking

int LinkProgramViaCode(const char **vertexCode, const char **pixelCode) {
	int vshader = CompileShaderViaCode(vertexCode, GL_VERTEX_SHADER);
	int pshader = CompileShaderViaCode(pixelCode, GL_FRAGMENT_SHADER);
	return LinkProgram(vshader, pshader);
}

int LinkProgramViaCode(const char **vertexCode, const char **tessellationControlCode, const char **tessellationEvalCode, const char **geometryCode, const char **pixelCode) {
	int vshader = CompileShaderViaCode(vertexCode, GL_VERTEX_SHADER);
	int tcshader = tessellationControlCode? CompileShaderViaCode(tessellationControlCode, GL_TESS_CONTROL_SHADER) : -1;
	int teshader = tessellationEvalCode? CompileShaderViaCode(tessellationEvalCode, GL_TESS_EVALUATION_SHADER) : -1;
	int gshader = geometryCode? CompileShaderViaCode(geometryCode, GL_GEOMETRY_SHADER) : -1;
	int pshader = CompileShaderViaCode(pixelCode, GL_FRAGMENT_SHADER);
	return LinkProgram(vshader, tcshader, teshader, gshader, pshader);
}

int LinkProgram(int vshader, int pshader) {
	return LinkProgram(vshader, -1, -1, -1, pshader);
}

int LinkProgram(int vshader, int tcshader, int teshader, int gshader, int pshader) {
    int programId = 0;
    // create shader program
    if (vshader && pshader)
        programId = glCreateProgram();
    if (programId > 0) {
        // attach shaders to program
        glAttachShader(programId, vshader);
		if (tcshader > 0)
			glAttachShader(programId, tcshader);
		if (teshader > 0)
			glAttachShader(programId, teshader);
		if (gshader > 0)
			glAttachShader(programId, gshader);
        glAttachShader(programId, pshader);
        // link and verify
        glLinkProgram(programId);
        GLint status;
        glGetProgramiv(programId, GL_LINK_STATUS, &status);
        if (status == GL_FALSE)
            PrintProgramLog(programId);
    }
    return programId;
}

int LinkProgramViaFile(const char *vertexShaderFile, const char *pixelShaderFile) {
	int vshader = CompileShaderViaFile(vertexShaderFile, GL_VERTEX_SHADER);
	int fshader = CompileShaderViaFile(pixelShaderFile, GL_FRAGMENT_SHADER);
	return LinkProgram(vshader, fshader);
}

int CurrentShader() {
	int shader = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shader);
	return shader;
}

// Uniform Access

bool Bad(bool report, const char *name) {
	if (report)
		printf("can't find named uniform: %s\n", name);
	return false;
}

bool SetUniform(int shader, const char *name, int val, bool report) {
	GLint id = glGetUniformLocation(shader, name);
	if (id < 0)
		return Bad(report, name);
	glUniform1i(id, val);
	return true;
}

bool SetUniformv(int shader, const char *name, int count, int *v, bool report) {
	GLint id = glGetUniformLocation(shader, name);
	if (id < 0)
		return Bad(report, name);
	glUniform1iv(id, count, v);
	return true;
}

bool SetUniform(int shader, const char *name, float val, bool report) {
	GLint id = glGetUniformLocation(shader, name);
	if (id < 0)
		return Bad(report, name);
	glUniform1f(id, val);
	return true;
}

bool SetUniform(int shader, const char *name, vec2 v, bool report) {
	GLint id = glGetUniformLocation(shader, name);
	if (id < 0)
		return Bad(report, name);
	glUniform2f(id, v.x, v.y);
	return true;
}

bool SetUniform(int shader, const char *name, vec3 v, bool report) {
	GLint id = glGetUniformLocation(shader, name);
	if (id < 0)
		return Bad(report, name);
	glUniform3f(id, v.x, v.y, v.z);
	return true;
}

bool SetUniform(int shader, const char *name, vec3 *v, bool report) {
	GLint id = glGetUniformLocation(shader, name);
	if (id < 0)
		return Bad(report, name);
	glUniform3fv(id, 1, (float *) v);
	return true;
}

bool SetUniform(int shader, const char *name, vec4 *v, bool report) {
	GLint id = glGetUniformLocation(shader, name);
	if (id < 0)
		return Bad(report, name);
	glUniform4fv(id, 1, (float *) v);
	return true;
}

bool SetUniform3(int shader, const char *name, float *v, bool report) {
	GLint id = glGetUniformLocation(shader, name);
	if (id < 0)
		return Bad(report, name);
	glUniform3fv(id, 1, v);
	return true;
}

bool SetUniform3v(int shader, const char *name, int count, float *v, bool report) {
	GLint id = glGetUniformLocation(shader, name);
	if (id < 0)
		return Bad(report, name);
	glUniform3fv(id, count, v);
	return true;
}

bool SetUniform(int shader, const char *name, mat4 m, bool report) {
	GLint id = glGetUniformLocation(shader, name);
	if (id < 0)
		return Bad(report, name);
	glUniformMatrix4fv(id, 1, true, (float *) &m[0][0]);
	return true;
}

// Attribute Access

void DisableVertexAttribute(int shader, const char *name) {
	GLint id = glGetAttribLocation(shader, name);
	if (id >= 0)
		glDisableVertexAttribArray(id);
}

int EnableVertexAttribute(int shader, const char *name) {
	GLint id = glGetAttribLocation(shader, name);
	if (id >= 0)
		glEnableVertexAttribArray(id);
	return id;
}

void VertexAttribPointer(int shader, const char *name, GLint ncomponents, GLenum datatype, GLboolean normalized, GLsizei stride, const GLvoid *pointer) {
	GLuint id = EnableVertexAttribute(shader, name);
	if (id < 0)
        printf("cant find attribute %s\n", name);
    glVertexAttribPointer(id, ncomponents, datatype, normalized, stride, pointer);
}
