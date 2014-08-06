/*
	A simple OpenGL library to load .obj files and display them without texture
	Usage:
	Declare a variable that contains the model
		Model myModel;
	Load the file
		loadOBJToModel("path/to/model.obj", &myModel);
	Then display the model
		drawModel(&myModel);
	Done.

	IMPORTANT NOTE: All faces must be triangles (I'll add support for rectangles later)

*/


#include <stdio.h>
#include <GL/glut.h>
#include "bitmap.h"

// A 3 double vector
typedef struct {
	GLdouble x, y, z;
} vector3d;

// A 3 int vector
typedef struct {
	int x, y, z;
} vector3i;

// A simple C implementation of std::vector class (This is not mine, I adapted one for this project)
// Arrayv is used for vertices, texture coordonates and normals
typedef struct {
	vector3d * array;
	size_t used;
	size_t size;
} Arrayv;

// Arrayf is used for faces
typedef struct {
	vector3i * array1; // 3 int that represent which vertice/texture coordonates/normal to use for the fist point
	vector3i * array2; // Same for the second point
	vector3i * array3; // And third
	size_t used;
	size_t size;
} Arrayf;

void initArrayv(Arrayv *a, size_t initialSize);
void insertArrayv(Arrayv *a, vector3d element);
void freeArrayv(Arrayv *a);

void initArrayf(Arrayf *a, size_t initialSize);
void insertArrayf(Arrayf *a, vector3i element[3]);
void freeArrayf(Arrayf *a);

// Model structure
typedef struct {
	// List of vertices
	Arrayv v;
	// Texture coordinates
	Arrayv vt;
	// Normals
	Arrayv vn;
	// Face Definitions
	Arrayf f;

} Model;

void loadOBJToModel(char * filename, Model * model);
void drawModel(Model * model);