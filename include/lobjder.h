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


// A 3 float vector
typedef struct {
	GLfloat x, y, z;
} vector3f;

// A 3 int vector
typedef struct {
	unsigned int x, y, z;
} vector3i;

// Arrayv is used for vertices, texture coordonates and normals
typedef struct {
	vector3f * array;
	size_t used;
	size_t size;
} Arrayv;

// Arrayf is used for faces
typedef struct {
	vector3i* array[4];
	size_t used;
	size_t size;
} Arrayf;

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
	vector3f offset; // Texture offset, not used
	vector3f scale; // Texture scale, not used
	int illum; // not used

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

// Sets paths to search for models, textures and materials
void setPaths(char * modelsFolderPath, char * materialsFolderPath, char * texturesFolderPath);

// Loads a .obj file to a model
void loadOBJToModel(char * fileName, Model * model);

// Draws the model to the scene using immediate mode
void drawModel(Model * model);

// Loads a .mtl file to a material array
void loadMTLToMaterials(char * fileName, Arraym * mat, int init); // init = 0 will append all materials found to the array, init = 1 will initialize the array

// Loads a material to be used for drawing
void loadMaterial(Material * mat);

// Loads a default material to be used for drawing
void loadDefaultMaterial();