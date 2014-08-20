#include <GL/glew.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/utility.h"

void normalizev(vector *v) {

	GLdouble modul = sqrt((*v).x * (*v).x + (*v).y * (*v).y + (*v).z * (*v).z);
	if (modul != 0) {
		(*v).x /= modul;
		(*v).y /= modul;
		(*v).z /= modul; 
	}
}

void multiplyv(vector *v, GLdouble factor) 
{
	(*v).x *= factor;
	(*v).y *= factor;
	(*v).z *= factor;
}

vector addv(vector v1, vector v2) 
{
	vector sum;
	sum.x = v1.x + v2.x;
	sum.y = v1.y + v2.y;
	sum.z = v1.z + v2.z;
	return sum;
}

vector substractv(vector v1, vector v2) 
{
	vector diff;
	diff.x = v1.x - v2.x;
	diff.y = v1.y - v2.y;
	diff.z = v1.z - v2.z;
	return diff;
}

vector rotatev(vector v, GLdouble angle, GLdouble x, GLdouble y, GLdouble z) 
{
	// Normalizes the rotation vector
	vector n = createv(x, y, z);
	normalizev(&n);
	x = n.x;
	y = n.y;
	z = n.z;

	GLdouble r = angle * DEG_TO_RAD;
	GLdouble c = cos(r);
	GLdouble s = sin(r);

	// Rotates the vector
	vector out;
	out.x = (x * x * (1 - c) + c) * v.x + (x * y * (1 - c) - z * s) * v.y + (x * z * (1 - c) + y * s) * v.z;
	out.y = (y * x * (1 - c) + z * s) * v.x + (y * y * (1 - c) + c) * v.y + (y * z * (1 - c) - x * s) * v.z;
	out.z = (x * z * (1 - c) - y * s) * v.x + (y * z * (1 - c) + x * s) * v.y + (z * z * (1 - c) + c) * v.z;

	return out;
}

vector createv(GLdouble x, GLdouble y, GLdouble z) 
{
	vector v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

GLdouble vlength(vector v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

void printv(vector v) {
	printf("%f %f %f\n", v.x, v.y, v.z);
}

// Reads all the content of file, creates a shader, adds the source and compiles it. Returns shader's id
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
	if (success == GL_FALSE) {
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