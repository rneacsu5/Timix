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
		Note: model.obj must be in "./path/to/model/", model.mtl must be in "./path/to/material/" and other .jpg, .png, .bmp etc. files in "./path/to/texture/"

	Note: Both triangle and quad faces are supported

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
} lbj_Model;

// Sets paths to search for models, textures and materials
void lbj_SetPaths(char * modelsFolderPath, char * materialsFolderPath, char * texturesFolderPath);

// Loads a .obj file to a model
void lbj_LoadOBJToModel(char * fileName, lbj_Model * model);

// Draws the model to the scene using immediate mode
void lbj_DrawModel(lbj_Model * model);

// Loads a .mtl file to a material array
void lbj_LoadMTLToMaterials(char * fileName, lbj_Arraym * mat, int init); // init = 0 will append all materials found to the array, init = 1 will initialize the array

// Loads a material to be used for drawing
void lbj_LoadMaterial(lbj_Material * mat);

// Loads a default material to be used for drawing
void lbj_LoadDefaultMaterial();

// Set up flipping: 1 = flip, 0 = don't flip, other = leave unchanged
void lbj_SetFlipping(int _flipU,     // Flip texture horizontally
				int _flipV,     // Flip texture vertically
				int _flipX,     // Flip model on the x axis
				int _flipY,     // Flip model on the y axis
				int _flipZ);    // Flip model on the z axis