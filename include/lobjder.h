/*
	A simple OpenGL library to load .obj files and display them without texture
	Usage:
	
		Declare a variable that contains the model

			Model myModel;

		Set the paths to model, material and  textures

			setPaths("./path/to/model/", "./path/to/material/", "./path/to/texture/");

		Load the file

			loadOBJToModel("model.obj", &myModel);

		Then display the model

			drawModel(&myModel);

		Done.
		Note: model.obj must be in "./path/to/model/", model.mtl must be in "./path/to/material/" and other .bmp files in "./path/to/texture/"

	IMPORTANT NOTE: Only .bmp files are supported
	Note: Both triangle and quad faces are supported

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
	unsigned int x, y, z;
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
	vector3i* array[4];
	size_t used;
	size_t size;
} Arrayf;


void initArrayv(Arrayv *a, size_t initialSize);
void insertArrayv(Arrayv *a, vector3d element);
void freeArrayv(Arrayv *a);

void initArrayf(Arrayf *a, size_t initialSize);
void insertArrayf(Arrayf *a, vector3i element[4]);
void freeArrayf(Arrayf *a);


// Material structure
typedef struct {
	char fileName[64]; // .bmp file name
	GLubyte* texData; // Texture data (extracted from the .bmp file using bitmap.h)
	BITMAPINFO* texInfo; // Texture info (extracted from the .bmp file using bitmap.h)
	GLuint glTexName; // Texture name: used with glBindTexture(GL_TEXTURE_2D, texName) to swich to diffrent textures
	GLsizei texWidth; // Texture width
	GLsizei texHeight; // Texture height
	char matName[64]; // Material name
	GLfloat Ka[3]; // Material Ambient color
	GLfloat Kd[3]; // Material Diffuse color
	GLfloat Ks[3]; // Material Specular color
	GLfloat Ns; // Material Shininess. Ranges form 0 to 1000
	GLfloat Tr; // Material Transparency. 1 = fully opaque 0 = fully transparent
	vector3d offset; // Texture offset;
	vector3d scale; // Texture scale;
	int illum; // Not used rigth now

} Material;

// Material array
typedef struct {
	Material * array;
	size_t used;
	size_t size;
} Arraym;

// Material index array
typedef struct {
	unsigned int * array;
	size_t used;
	size_t size;
} Arraymi;

void initArraym(Arraym *a, size_t initialSize);
void insertArraym(Arraym *a, Material mat);
void freeArraym(Arraym *a);

void initArraymi(Arraymi *a, size_t initialSize);
void insertArraymi(Arraymi *a, unsigned int mat);
void freeArraymi(Arraymi *a);

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
	// Materials used
	Arraym mats;
	// Material index
	Arraymi matsi;
} Model;

void setPaths(char * modelsFolderPath, char * materialsFolderPath, char * texturesFolderPath);
void loadOBJToModel(char * fileName, Model * model);
void drawModel(Model * model);
void loadMTLToMaterials(char * fileName, Arraym * mat);
void loadMaterial(Material * mat);