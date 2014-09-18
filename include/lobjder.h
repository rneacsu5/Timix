/*

	lobjder library 
	Copyright (c) 2014 Neacsu Razvan

	A simple OpenGL library to load .obj files and display them
	Usage:
	
		Declare a variable that contains the model

			lbj_Model myModel;

		Set the paths to model, material and  textures

			lbj_SetPaths("./path/to/model/", "./path/to/material/", "./path/to/texture/");

		Load the file

			lbj_LoadOBJToModel("model.obj", &myModel);
			lbj_CreateVBO(&myModel, 0); // << Optional if you don't want to use VBOs

		Then display the model using immediate mode

			lbj_DrawModelIM(myModel);

		or using VBO (must create the VBO before)

			lbj_DrawModelVBO(myModel);

		Done.
		Note: model.obj must be in "./path/to/model/", model.mtl must be in "./path/to/material/" and other .jpg, .png, .bmp etc. files in "./path/to/texture/"

	Note: Triangle and quad faces are supported. N-GONS are NOT supported
	Note: stb_image is used for loading textures. See http://nothings.org/stb_image.c for more info

*/

#include <stdio.h>
#include <GL/glut.h>


// A 3 float vector
typedef struct {
	GLfloat x, y, z;
} lbj_vector3f;

// A 3 int vector
typedef struct {
	unsigned int x, y, z;
} lbj_vector3i;

// lbj_Arrayv is used for vertices, texture coordonates and normals
typedef struct {
	lbj_vector3f * array;
	size_t used;
	size_t size;
} lbj_Arrayv;

// lbj_Arrayf is used for faces
typedef struct {
	lbj_vector3i* array[4];
	size_t used;
	size_t size;
} lbj_Arrayf;

// Material structure
typedef struct {
	char * fileName; // File name
	unsigned char* texData; // Texture data
	int texWidth; // Texture width
	int texHeight; // Texture height
	GLuint glTexName; // Texture name: used with glBindTexture(GL_TEXTURE_2D, texName) to swich to diffrent textures
	char * matName; // Material name
	GLfloat Ka[3]; // Material Ambient color
	GLfloat Kd[3]; // Material Diffuse color
	GLfloat Ks[3]; // Material Specular color
	GLfloat Ns; // Material Shininess. Ranges form 0 to 1000
	GLfloat Tr; // Material Transparency. 1 = fully opaque 0 = fully transparent
	lbj_vector3f offset; // Texture offset, not used
	lbj_vector3f scale; // Texture scale, not used
	int illum; // not used

} lbj_Material;

// Material array
typedef struct {
	lbj_Material * array;
	size_t used;
	size_t size;
} lbj_Arraym;

// Material index array
typedef struct {
	unsigned int * array;
	size_t used;
	size_t size;
} lbj_Arraymi;

// Model structure
typedef struct {
	// List of vertices
	lbj_Arrayv v;
	// Texture coordinates
	lbj_Arrayv vt;
	// Normals
	lbj_Arrayv vn;
	// Face Definitions
	lbj_Arrayf f;
	// Materials used
	lbj_Arraym mats;
	// Material index
	lbj_Arraymi matsi;
	// VBO vertex Buffer ID
	GLuint vertexBuffID;
	// VBO index Buffer ID
	GLuint indexBuffID;
} lbj_Model;

// A vertex structure used for VBO
typedef struct {
	// Position
	GLfloat pos[3];
	// Normal
	GLfloat normal[3];
	// Texture coordonates
	GLfloat texCoord[2];
} lbj_VBOVertex;

// Sets paths to search for models, textures and materials
void lbj_SetPaths(char * modelsFolderPath, char * materialsFolderPath, char * texturesFolderPath);

// Loads a .obj file to a model
void lbj_LoadOBJToModel(char * fileName, lbj_Model * model);

// Draws the model to the scene using immediate mode
void lbj_DrawModelIM(lbj_Model model);

// Loads a .mtl file to a material array
void lbj_LoadMTLToMaterials(char * fileName, lbj_Arraym * mat, int init); // init = 0 will append all materials found to the array, init = 1 will initialize the array

// Loads a material to be used for drawing
void lbj_LoadMaterial(lbj_Material mat);

// Loads a default material to be used for drawing
void lbj_LoadDefaultMaterial();

// Set up flipping: 1 = flip, 0 = don't flip, other = leave unchanged
void lbj_SetFlipping(int _flipU,     // Flip texture horizontally
					int _flipV,     // Flip texture vertically
					int _flipX,     // Flip model on the x axis
					int _flipY,     // Flip model on the y axis
					int _flipZ);    // Flip model on the z axis

// Creates and populates a VBO for the model
// Set "economic" to 1 if you want to reuse vertices or to 0 if you don't want to
// IMPORTANT: Don't use economic with large models: IT'S EXTREMELY SLOW
void lbj_CreateVBO(lbj_Model * model, int economic);

// Draws the model using VBO
void lbj_DrawModelVBO(lbj_Model model);