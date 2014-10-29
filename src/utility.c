#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "../include/utility.h"

// Reads all the content of file, creates a shader, adds the source and compiles it. Returns the shader's id
GLuint loadShaderFromFile(char * path, GLenum type) {
	// Creates Shader
	GLuint id = glCreateShader(type);

	// Opens the file
	FILE * fp = fopen(path, "rb");
	if (fp == NULL) {
		printf("ERROR: Failed to open \"%s\".\n", path);
		return id;
	}

	// Gets file length
	int length;
	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Allocates memory and loads file's content
	char * code;
	code = (char *) malloc(length * sizeof(char));
	fread(code, length * sizeof(char), 1, fp);

	// Close file
	fclose(fp);

	// Adds shader source and compiles it
	glShaderSource(id, 1, (const GLchar **) &code, NULL);
	glCompileShader(id);

	// Warns the user if the compilation failed
	GLint success = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (success != GL_TRUE) {
		printf("WARNING: Shader at \"%s\" didn't compile\n", path);
	}
	else {
		//printf("Shader at \"%s\" compiled successfully\n", path);
	}
	return id;
}

// Creates a program and links two shaders. Returns the program's id.
GLuint createShadersProgram(GLuint id1, GLuint id2) {
	GLuint id = glCreateProgram();
	glAttachShader(id, id1);
	glAttachShader(id, id2);
	glLinkProgram(id);
	return id;
}

// Loads two Shaders from files
void loadShaders(char * path1, GLenum type1, char * path2, GLenum type2) {
	GLuint id1 = loadShaderFromFile(path1, type1);
	GLuint id2 = loadShaderFromFile(path2, type2);
	GLuint programID = createShadersProgram(id1, id2);
	glUseProgram(programID);
}