/*

	lobjder library 
	Copyright (c) 2014 Neacsu Razvan

	A simple OpenGL library to load .obj files and display them

	Use this line before you include this header to create the implementation. USE IT ONLY ONCE!
		#define LOBJDER_IMPLEMENTATION
	Also add #define LOBJDER_COLOR_OUTPUT to have the output colored. "colortext" library is required.

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
	Note: Implementation for stb_image is created in this header file by default. If you want to prevent that use "#define LBJ_NO_STB_IMPLEMENTATION".
	Note: Everything prefixed with "lbjp_" or "LBJP_" (stands for "lobjder private") should not be used or else problems might occur.

*/

#ifndef LOBJDER_H_INCLUDED

#define LOBJDER_H_INCLUDED

#include <stdio.h>
#include <GL/glut.h>


// A 3 float vector
typedef struct {
	GLfloat x, y, z;
} lbj_Vector3f;

// A 3 int vector
typedef struct {
	unsigned int x, y, z;
} lbj_Vector3i;

// lbj_Arrayv is used for vertices, texture coordonates and normals
typedef struct {
	lbj_Vector3f * array;
	unsigned int used;
	unsigned int size;
} lbj_Arrayv;

// lbj_Arrayf is used for faces
typedef struct {
	lbj_Vector3i* array[4];
	unsigned int used;
	unsigned int size;
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
	lbj_Vector3f offset; // Texture offset, not used
	lbj_Vector3f scale; // Texture scale, not used
	int illum; // not used
} lbj_Material;

// Material array
typedef struct {
	lbj_Material * array;
	unsigned int used;
	unsigned int size;
} lbj_Arraym;

// Material index array
typedef struct {
	unsigned int * array;
	unsigned int used;
	unsigned int size;
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

// Whether to print stats about the model or just warnings
void lbj_PrintStats(int value);


// Implementation
#ifdef LOBJDER_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES

#ifndef LBJ_NO_STB_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#endif // LBJ_NO_STB_IMPLEMENTATION
#include "stb_image.h"

#ifdef LOBJDER_COLOR_OUTPUT
#include "colortext.h"
#endif // LOBJDER_COLOR_OUTPUT

// Paths
static char * lbjp_modelsPath = NULL;
static char * lbjp_materialsPath = NULL;
static char * lbjp_texturesPath = NULL;
// A neutral, gray material
static lbj_Material lbjp_defaultMaterial;
static int lbjp_firstUsed = 1;
// Used for flipping the model or texture
static int lbjp_flipU = 0, lbjp_flipV = 1, lbjp_flipX = 0, lbjp_flipY = 0, lbjp_flipZ = 0;
// Print stats or not
static int lbjp_printStats = 0;

// Initializes the Array and allocates memory
static void lbjp_initArrayv(lbj_Arrayv *a, unsigned int initialSize)
{
	if (initialSize <= 0)
		a->size = 1;
	else
		a->size = initialSize;
	a->array = (lbj_Vector3f *)malloc(a->size * sizeof(lbj_Vector3f));
	a->used = 0;
}

// Adds an element to the array and reallocates memory if needed
static void lbjp_insertArrayv(lbj_Arrayv *a, lbj_Vector3f element)
{
	if (a->used == a->size) {
		a->size *= 2;
		a->array = (lbj_Vector3f *)realloc(a->array, a->size * sizeof(lbj_Vector3f));
	}
	a->array[a->used++] = element;
}

// Frees the memory and resets the array
static void lbjp_freeArrayv(lbj_Arrayv *a)
{
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

// Same as above but for a face array
static void lbjp_initArrayf(lbj_Arrayf *a, unsigned int initialSize)
{
	int i;
	if (initialSize <= 0)
		a->size = 1;
	else
		a->size = initialSize;
	for (i = 0; i < 4; i++) {
		a->array[i] = (lbj_Vector3i *)malloc(a->size * sizeof(lbj_Vector3i));
	}
	a->used = 0;
}

static void lbjp_insertArrayf(lbj_Arrayf *a, lbj_Vector3i element[4])
{
	int i;
	if (a->used == a->size) {
		a->size *= 2;
		for (i = 0; i < 4; i++) {
			a->array[i] = (lbj_Vector3i *)realloc(a->array[i], a->size * sizeof(lbj_Vector3i));
		}
	}
	for (i = 0; i < 4; i++) {
		a->array[i][a->used] = element[i];
	}
	a->used++;
}

static void lbjp_freeArrayf(lbj_Arrayf *a)
{
	int i;
	for (i = 0; i < 4; i++) {
		free(a->array[i]);
		a->array[i] = NULL;
	}
	a->used = a->size = 0;
}

// For a materials array
static void lbjp_initArraym(lbj_Arraym *a, unsigned int initialSize)
{
	if (initialSize <= 0)
		a->size = 1;
	else
		a->size = initialSize;
	a->array = (lbj_Material *)malloc(a->size * sizeof(lbj_Material));
	a->used = 0;
}

static void lbjp_insertArraym(lbj_Arraym *a, lbj_Material mat)
{
	if (a->used == a->size) {
		a->size *= 2;
		a->array = (lbj_Material *)realloc(a->array, a->size * sizeof(lbj_Material));
	}
	a->array[a->used++] = mat;
}

static void lbjp_freeArraym(lbj_Arraym *a)
{
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

// For a material indexes array
static void lbjp_initArraymi(lbj_Arraymi *a, unsigned int initialSize)
{
	if (initialSize <= 0)
		a->size = 1;
	else
		a->size = initialSize;
	a->array = (unsigned int *)malloc(a->size * sizeof(unsigned int));
	a->used = 0;
}

static void lbjp_insertArraymi(lbj_Arraymi *a, unsigned int mat)
{
	if (a->used == a->size) {
		a->size *= 2;
		a->array = (unsigned int *)realloc(a->array, a->size * sizeof(unsigned int));
	}
	a->array[a->used++] = mat;
}

static void lbjp_freeArraymi(lbj_Arraymi *a)
{
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

// Sets paths to look for files
void lbj_SetPaths(char * modelsFolderPath, char * materialsFolderPath, char * texturesFolderPath)
{
	lbjp_modelsPath = modelsFolderPath;
	lbjp_materialsPath = materialsFolderPath;
	lbjp_texturesPath = texturesFolderPath;
}

// Sets flipping
void lbj_SetFlipping(int _flipU, int _flipV, int _flipX, int _flipY, int _flipZ)
{
	// If set to 0 : no flipping; 1 : flip; other: leave it unchanged

	if (_flipU == 0 || _flipU == 1) lbjp_flipU = _flipU;
	if (_flipV == 0 || _flipV == 1) lbjp_flipV = _flipV;
	if (_flipX == 0 || _flipX == 1) lbjp_flipX = _flipX;
	if (_flipY == 0 || _flipY == 1) lbjp_flipY = _flipY;
	if (_flipZ == 0 || _flipZ == 1) lbjp_flipZ = _flipZ;
}

// Loads a .obj file to a model
void lbj_LoadOBJToModel(char * fileName, lbj_Model * model)
{
	// Opens file in read-text mode
	char * path;
	if (lbjp_modelsPath != NULL) {
		path = (char *)malloc((strlen(lbjp_modelsPath) + strlen(fileName) + 1) * sizeof(char));
		strcpy(path, lbjp_modelsPath);
		strcat(path, fileName);
	}
	else {
		path = (char *)malloc((strlen(fileName) + 1) * sizeof(char));
		strcpy(path, fileName);
	}
	FILE * fp = fopen(path, "rt");
	if (fp == NULL) {

#ifdef LOBJDER_COLOR_OUTPUT
		ctxt_PrintColored(ctxt_CreateColor(CTXT_BLACK, CTXT_LIGHT_YELLOW, CTXT_DEFAULT), "WARNING: Failed to open \"%s\".", path);
		printf("\n");
#else
		printf("WARNING: Failed to open \"%s\".\n", path);
#endif // LOBJDER_COLOR_OUTPUT

		return;
	}

	// Initializes the model's arrays
	lbjp_initArrayv(&model->v, 10);
	lbjp_initArrayv(&model->vt, 10);
	lbjp_initArrayv(&model->vn, 10);
	lbjp_initArrayf(&model->f, 10);
	lbjp_initArraymi(&model->matsi, 5);
	lbjp_initArraym(&model->mats, 5);

	// Now we need to read the file line by line
	char * line = (char *)malloc(128 * sizeof(char));
	int matIndex = 0;
	lbj_Vector3f vect;
	while (fgets(line, 128, fp)) {
		// The v flag is a vertice
		if (strncmp(line, "v ", 2) == 0) {
			if (sscanf(line, "%*s %f %f %f", &vect.x, &vect.y, &vect.z) == 3) {
				if (lbjp_flipX) vect.x = -vect.x;
				if (lbjp_flipY) vect.y = -vect.y;
				if (lbjp_flipZ) vect.z = -vect.z;
				lbjp_insertArrayv(&model->v, vect);
			}
		}

		// The vt flag is a texture coordonate
		else if (strncmp(line, "vt", 2) == 0) {
			vect.z = 0;
			if (sscanf(line, "%*s %f %f", &vect.x, &vect.y) == 2) {
				if (lbjp_flipU) vect.x = -vect.x;
				if (lbjp_flipV) vect.y = -vect.y;
				lbjp_insertArrayv(&model->vt, vect);
			}
		}

		// The vn flag is a normal
		else if (strncmp(line, "vn", 2) == 0) {
			if (sscanf(line, "%*s %f %f %f", &vect.x, &vect.y, &vect.z) == 3) {
				if (lbjp_flipX) vect.x = -vect.x;
				if (lbjp_flipY) vect.y = -vect.y;
				if (lbjp_flipZ) vect.z = -vect.z;
				lbjp_insertArrayv(&(*model).vn, vect);
			}
		}

		// The f flag is a face
		else if (strncmp(line, "f ", 2) == 0) {
			lbj_Vector3i vct[4]; // Output vector
			char * point = (char *)malloc(sizeof(char)); // A vertice/texture/normal sequence

			// Note that there are four formats:
			// <1>     f 145 679 12
			// <2>     f 145/32 679/45 12/64
			// <3>     f 145/32/457 679/45/4799 12/64/146
			// <4>     f 145//457 679//4799 12//146

			// We need to analyze each one

			int i;
			char * p[5]; // Pointers to blank spaces
			int fail = 0; // Used to detect failures in reading line
			p[0] = strchr(line, 32);
			for (i = 0; i < 4; i++) {
				if (p[i] == NULL) {
					if (i <= 2) {
						fail++;
					}
					else if (i == 3) {
						vct[3] = vct[2];
					}
					break;
				}
				p[i + 1] = strchr(p[i] + 1, 32);
				if (p[i + 1] != NULL) {
					point = (char *)realloc(point, (p[i + 1] - p[i]) * sizeof(char));
					strncpy(point, p[i] + 1, p[i + 1] - p[i] - 1); // Take a sequence
					point[p[i + 1] - p[i] - 1] = '\0'; // Add the null character
				}
				else {
					point = (char *)realloc(point, (strlen(p[i] + 1) + 1) * sizeof(char));
					strcpy(point, p[i] + 1);
				}
				char * c1; // Pointer to first forward-slash
				char * c2; // Pointer to second forward-slash
				// These two pointers can be NULL so this is how we figure out the format used

				c1 = strchr(point, '/'); // Check for any forward-slash
				if (c1 == NULL) {
					// None found. This is format <1>
					if (sscanf(point, "%d", &vct[i].x) != 1)
						fail++;
					vct[i].y = vct[i].z = 0;
				}
				else {
					// Found the first one. Lets check for the second
					c2 = strchr(c1 + 1, '/');
					if (c2 == NULL) {
						// Second one is missing. Format <2>
						if (sscanf(point, "%d/%d", &vct[i].x, &vct[i].y) != 2)
							fail++;
						vct[i].z = 0;
					}
					else {
						// Found the second one
						// Let's check if they are next to each other
						if (c2 == c1 + 1) {
							// Format <4>
							if (sscanf(point, "%d//%d", &vct[i].x, &vct[i].z) != 2)
								fail++;
							vct[i].y = 0;
						}
						else {
							// Format <3>
							if (sscanf(point, "%d/%d/%d", &vct[i].x, &vct[i].y, &vct[i].z) != 3)
								fail++;
						}
					}
				}
			}

			free(point);

			if (fail == 0) {
				lbjp_insertArrayf(&model->f, vct);
				lbjp_insertArraymi(&model->matsi, matIndex);
			}
		}
		// .mtl file reference
		else if (strncmp(line, "mtllib", 6) == 0) {
			char * name = (char *)malloc((strlen(line) - 6) * sizeof(char));
			if (sscanf(line, "%*s %s", name) == 1)
				lbj_LoadMTLToMaterials(name, &model->mats, 0);
			free(name);
		}
		// The usemtl flag is a material
		else if (strncmp(line, "usemtl", 6) == 0) {
			char * name = (char *)malloc((strlen(line) - 6) * sizeof(char));
			unsigned int i;
			if (sscanf(line, "%*s %s", name) == 1) {
				for (i = 0; i < model->mats.used; i++) {
					if (strcmp(name, model->mats.array[i].matName) == 0) {
						matIndex = i;
						break;
					}
				}
			}
			free(name);
		}
		else {
			// Found something else. Maybe a comment?
		}
	}

	if (lbjp_printStats) {
		// Print Stats

#ifdef LOBJDER_COLOR_OUTPUT
		ctxt_PrintColored(ctxt_CreateColor(CTXT_LIGHT_GREEN, CTXT_DEFAULT, CTXT_UNDERLINED), "MODEL:\n");
#else
		printf("MODEL:\n");
#endif // LOBJDER_COLORED_OUTPUT

		printf("Loaded \"%s\".\n", path);
		printf("Contained: %u vertice, %u texture coord, %u normals, %u faces, %u materials\n",
			   model->v.used,
			   model->vt.used,
			   model->vn.used,
			   model->f.used,
			   model->mats.used);
		printf("Allocated %u bytes of memory\n",
			   model->v.size * sizeof(lbj_Vector3f) +
			   model->vt.size * sizeof(lbj_Vector3f) +
			   model->vn.size * sizeof(lbj_Vector3f) +
			   model->f.size * sizeof(lbj_Vector3i) * 4 +
			   model->mats.size * sizeof(lbj_Material) +
			   model->matsi.size * sizeof(unsigned int));
		printf("\n");
	}

	// Free memory
	free(line);
	free(path);

	// Close file
	fclose(fp);
}

// Loads a .mtl file to a material array
void lbj_LoadMTLToMaterials(char * fileName, lbj_Arraym * mats, int init)
{
	// Opens file in read-text mode
	char * path;
	if (lbjp_materialsPath != NULL) {
		path = (char *)malloc((strlen(lbjp_materialsPath) + strlen(fileName) + 1) * sizeof(char));
		strcpy(path, lbjp_materialsPath);
		strcat(path, fileName);
	}
	else {
		path = (char *)malloc((strlen(fileName) + 1) * sizeof(char));
		strcpy(path, fileName);
	}
	FILE * fp = fopen(path, "rt");
	if (fp == NULL) {

#ifdef LOBJDER_COLOR_OUTPUT
		ctxt_PrintColored(ctxt_CreateColor(CTXT_BLACK, CTXT_LIGHT_YELLOW, CTXT_DEFAULT), "WARNING: Failed to open \"%s\".", path);
		printf("\n");
#else
		printf("WARNING: Failed to open \"%s\".\n", path);
#endif // LOBJDER_COLOR_OUTPUT

		return;
	}
	// Initialize the array if init != 0
	if (init != 0) {
		lbjp_initArraym(mats, 1);
	}

	// Now we need to read the file line by line
	int i = 0;
	lbj_Material mat;
	char * line = (char *)malloc(128 * sizeof(char));
	while (fgets(line, 128, fp)) {
		// The newmtl flag is a new material
		if (strncmp(line, "newmtl", 6) == 0) {
			if (i != 0) {
				lbjp_insertArraym(mats, mat);
			}
			i++;
			lbj_LoadDefaultMaterial();
			mat = lbjp_defaultMaterial;
			mat.matName = (char *)malloc((strlen(line) - 6) * sizeof(char));
			sscanf(line, "%*s %s", mat.matName);
			glGenTextures(1, &mat.glTexName);
			glBindTexture(GL_TEXTURE_2D, mat.glTexName);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		// Material ambient
		else if (strncmp(line, "Ka", 2) == 0) {
			sscanf(line, "%*s %f %f %f", &mat.Ka[0], &mat.Ka[1], &mat.Ka[2]);
		}
		// Material diffuse
		else if (strncmp(line, "Kd", 2) == 0) {
			sscanf(line, "%*s %f %f %f", &mat.Kd[0], &mat.Kd[1], &mat.Kd[2]);
		}
		// Material specular
		else if (strncmp(line, "Ks", 2) == 0) {
			sscanf(line, "%*s %f %f %f", &mat.Ks[0], &mat.Ks[1], &mat.Ks[2]);
		}
		// Material shininess
		else if (strncmp(line, "Ns", 2) == 0) {
			sscanf(line, "%*s %f", &mat.Ns);
		}
		// Material transparency
		else if (strncmp(line, "Tr", 2) == 0 || strncmp(line, "d ", 2) == 0) {
			sscanf(line, "%*s %f", &mat.Tr);
		}
		// Material illum
		else if (strncmp(line, "illum", 5) == 0) {
			sscanf(line, "%*s %d", &mat.illum);
		}
		// Texture file
		else if (strncmp(line, "map_Kd", 6) == 0) {
			// There are 2 options that we need to check: -o and -s
			char * p;
			p = strstr(line, "-o");
			if (p != NULL) {
				// Found -o option. This is texture offset
				sscanf(p, "%*s %f %f", &mat.offset.x, &mat.offset.y);
			}
			p = strstr(line, "-s");
			if (p != NULL) {
				// Found -s option. This is texture scale
				sscanf(p, "%*s %f %f", &mat.scale.x, &mat.scale.y);
			}
			// The file name is the last
			p = strchr(line, 32);
			char * p1 = strchr(p + 1, 32);
			while (p1 != NULL) {
				p = p1;
				p1 = strchr(p + 1, 32);
			}
			mat.fileName = (char *)malloc((strlen(p + 1) + 1) * sizeof(char));
			strcpy(mat.fileName, p + 1);

			unsigned int k;
			for (k = 0; k <= strlen(mat.fileName); k++) {
				if (mat.fileName[k] == '\r' || mat.fileName[k] == '\n') {
					// New line caracters found so we remove them
					mat.fileName[k] = '\0';
					break;
				}
			}

			// Full path
			char * path1;
			if (lbjp_texturesPath != NULL) {
				path1 = (char *)malloc((strlen(lbjp_texturesPath) + strlen(mat.fileName) + 1) * sizeof(char));
				strcpy(path1, lbjp_texturesPath);
				strcat(path1, mat.fileName);
			}
			else {
				path1 = (char *)malloc((strlen(mat.fileName) + 1) * sizeof(char));
				strcpy(path1, mat.fileName);
			}

			// Gets the texture data and texture info
			int n;
			mat.texData = stbi_load(path1, &mat.texWidth, &mat.texHeight, &n, 3);
			if (mat.texData == NULL) {

#ifdef LOBJDER_COLOR_OUTPUT
				ctxt_PrintColored(ctxt_CreateColor(CTXT_BLACK, CTXT_LIGHT_YELLOW, CTXT_DEFAULT), "WARNING: Failed to open \"%s\".\nstb_image.h says: %s", path1, stbi_failure_reason());
				printf("\n");
#else
				printf("WARNING: Failed to open \"%s\".\nstb_image.h says: %s\n", path1, stbi_failure_reason());
#endif // LOBJDER_COLOR_OUTPUT

			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
							 mat.texWidth,
							 mat.texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE,
							 mat.texData);
			}

			free(path1);
		}
		// Custom flag for pixelated textures
		else if (strncmp(line, "pixelated", 9) == 0) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else {
			// Found something else
		}
	}
	if (i != 0) {
		lbjp_insertArraym(mats, mat);
	}

	// Free memory
	free(line);
	free(path);

	// Close file
	fclose(fp);

}

// Loads a material to be used for drawing
void lbj_LoadMaterial(lbj_Material mat)
{
	GLfloat matAmbient[] = {mat.Ka[0], mat.Ka[1], mat.Ka[2], 1};
	GLfloat matDiffuse[] = {mat.Kd[0], mat.Kd[1], mat.Kd[2], mat.Tr};
	GLfloat matSpecular[] = {mat.Ks[0], mat.Ks[1], mat.Ks[2], 1};
	GLfloat matShininess[] = {mat.Ns * 128 / 1000};

	glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

	glBindTexture(GL_TEXTURE_2D, mat.glTexName);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

// Load a default material
void lbj_LoadDefaultMaterial(void)
{
	// The first time this function is called, initialize the "defaultMaterial" and load it
	// The next time the function is called, just load the "defaultMaterial"

	if (lbjp_firstUsed) {
		lbjp_firstUsed = 0;
		// Initializes the "defaultMaterial"
		lbjp_defaultMaterial.matName = (char *)malloc(8 * sizeof(char));
		lbjp_defaultMaterial.fileName = (char *)malloc(5 * sizeof(char));
		strcpy(lbjp_defaultMaterial.matName, "default");
		strcpy(lbjp_defaultMaterial.fileName, "none");
		glGenTextures(1, &lbjp_defaultMaterial.glTexName);
		glBindTexture(GL_TEXTURE_2D, lbjp_defaultMaterial.glTexName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		lbjp_defaultMaterial.Ka[0] = lbjp_defaultMaterial.Ka[1] = lbjp_defaultMaterial.Ka[2] = 0.2;
		lbjp_defaultMaterial.Kd[0] = lbjp_defaultMaterial.Kd[1] = lbjp_defaultMaterial.Kd[2] = 0.8;
		lbjp_defaultMaterial.Ks[0] = lbjp_defaultMaterial.Ks[1] = lbjp_defaultMaterial.Ks[2] = 1.0;
		lbjp_defaultMaterial.Ns = 50;
		lbjp_defaultMaterial.Tr = 1;
		lbjp_defaultMaterial.illum = 2;
		lbjp_defaultMaterial.offset.x = lbjp_defaultMaterial.offset.y = lbjp_defaultMaterial.offset.z = 0;
		lbjp_defaultMaterial.scale.x = lbjp_defaultMaterial.scale.y = lbjp_defaultMaterial.scale.z = 1;
	}

	// Load the default material
	lbj_LoadMaterial(lbjp_defaultMaterial);
}

// Draws model using immediate mode
void lbj_DrawModelIM(lbj_Model model)
{
	unsigned int i, j, a, b, c;
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	// For each face
	for (i = 0; i < model.f.used; i++) {
		// Determines whether or not to change the material
		if (model.mats.used > 0) {
			if (i == 0) {
				// This is the first face, load the first material
				glEnd();
				lbj_LoadMaterial(model.mats.array[model.matsi.array[i]]);
				glBegin(GL_QUADS);
			}
			else if (model.matsi.array[i - 1] != model.matsi.array[i]) {
				// The previous face had a diffrent material, load the next material
				glEnd();
				lbj_LoadMaterial(model.mats.array[model.matsi.array[i]]);
				glBegin(GL_QUADS);
			}
		}
		else if (i == 0) {
			// No materials for this model. Load default material
			glEnd();
			lbj_LoadDefaultMaterial();
			glBegin(GL_QUADS);
		}

		// Draw the face
		for (j = 0; j < 4; j++) {
			a = model.f.array[j][i].x; // Index of vertice
			b = model.f.array[j][i].y; // Index of texture coordonates
			c = model.f.array[j][i].z; // Index of normal

			if (c > 0) {
				c--; // Substact one because the first vertice,normal, etc. starts at 1 in an .obj file
				glNormal3f(model.vn.array[c].x, model.vn.array[c].y, model.vn.array[c].z);
			}
			if (b > 0) {
				b--;
				glTexCoord2f(model.vt.array[b].x, model.vt.array[b].y);
			}
			if (a > 0) {
				a--;
				glVertex3f(model.v.array[a].x, model.v.array[a].y, model.v.array[a].z);
			}
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

// Creates a VBO for the given model
void lbj_CreateVBO(lbj_Model * model, int economic)
{
	// Number of used elements in the vertex array and the allocated size for the array
	unsigned int usedVertices = 0, sizeAllocated = 4;

	// The data in the ARRAY BUFFER
	lbj_VBOVertex * vertices = (lbj_VBOVertex *)malloc(sizeAllocated * sizeof(lbj_VBOVertex));

	// The data in the ELEMENT ARRAY BUFFER
	GLint * indices = (GLint *)malloc(model->f.used * 4 * sizeof(GLint));

	// Some variables
	unsigned int i, j, k, a, b, c;

	// Used for displaying the procentage done
	int lastProcent = 0, procentLoaded = 0;

	// A variable to hold a VBOVertex 
	lbj_VBOVertex vert;

	if (lbjp_printStats && model->f.used != 0) 

#ifdef LOBJDER_COLOR_OUTPUT
		ctxt_PrintColored(ctxt_CreateColor(CTXT_LIGHT_MAGENTA, CTXT_DEFAULT, CTXT_DEFAULT), "Creating VBO...\n");
#else
		printf("Creating VBO...\n");
#endif // LOBJDER_COLOR_OUTPUT

	for (i = 0; i < model->f.used; i++) {
		for (j = 0; j < 4; j++) {
			a = model->f.array[j][i].x; // Index of vertice
			b = model->f.array[j][i].y; // Index of texture coordonates
			c = model->f.array[j][i].z; // Index of normal

			if (a > 0) {
				a--;
				vert.pos[0] = model->v.array[a].x;
				vert.pos[1] = model->v.array[a].y;
				vert.pos[2] = model->v.array[a].z;
			}
			if (c > 0) {
				c--;
				vert.normal[0] = model->vn.array[c].x;
				vert.normal[1] = model->vn.array[c].y;
				vert.normal[2] = model->vn.array[c].z;
			}
			if (b > 0) {
				b--;
				vert.texCoord[0] = model->vt.array[b].x;
				vert.texCoord[1] = model->vt.array[b].y;
			}

			int found = -1;

			// If "economic" we try to find a vertex in "vertices" that is the same as "vert". We reuse vertices therefore the ARRAY BUFFER is smaller
			// This is why using "economic" is SLOW AS HELL for big models
			if (economic) {
				for (k = 0; k < usedVertices; k++) {
					if (vert.pos[0] == vertices[k].pos[0] &&
						vert.pos[1] == vertices[k].pos[1] &&
						vert.pos[2] == vertices[k].pos[2] &&
						vert.normal[0] == vertices[k].normal[0] &&
						vert.normal[1] == vertices[k].normal[1] &&
						vert.normal[2] == vertices[k].normal[2] &&
						vert.texCoord[0] == vertices[k].texCoord[0] &&
						vert.texCoord[1] == vertices[k].texCoord[1]) {
						found = k;
						break;
					}
				}
			}

			// If not "economic" or the vertex is new we add it to the vertices array
			if (found == -1) {
				// Increment number of vertices
				usedVertices++;
				// Allocate memory if needed
				if (usedVertices > sizeAllocated) {
					sizeAllocated *= 2;
					vertices = (lbj_VBOVertex *)realloc(vertices, sizeAllocated * sizeof(lbj_VBOVertex));
				}
				// Add the vertex
				vertices[usedVertices - 1] = vert;
				// Add the index
				indices[4 * i + j] = usedVertices - 1;
			}
			// "vert" was previously added so we just add the index to the indices array
			else {
				indices[4 * i + j] = found;
			}
		}
		if (lbjp_printStats) {
			// A simple "loading screen"
			// Get the procentage done
			procentLoaded = (int)(i / (float)(model->f.used - 1) * 100);

			// Print it (the carrage return keeps it on the same line)
			if (procentLoaded == 100) {
				printf("\rLoaded: 100 %%\n");
				printf("\rThere are %u vertices in the vertex buffer and %u indices in the index buffer\n\n", usedVertices, model->f.used * 4);
			}
			else if (procentLoaded > lastProcent) {
				printf("\rLoaded: %d %%", procentLoaded);
				fflush(stdout);
				lastProcent = procentLoaded;
			}
		}
	}

	// Generate a vertex buffer
	glGenBuffers(1, &model->vertexBuffID);
	// Bind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexBuffID);
	// Add data
	glBufferData(GL_ARRAY_BUFFER, usedVertices * sizeof(lbj_VBOVertex), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// The same as above
	glGenBuffers(1, &model->indexBuffID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indexBuffID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->f.used * 4 * sizeof(GLint), indices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Draws the model using VBO
void lbj_DrawModelVBO(lbj_Model model)
{
	glEnable(GL_TEXTURE_2D);

	// Enables client states
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Binds the buffers
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexBuffID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffID);

	// Specifies the vertex, normal, texture coordonates pointers
	glVertexPointer(3, GL_FLOAT, sizeof(lbj_VBOVertex), (char*)NULL + 0);
	glNormalPointer(GL_FLOAT, sizeof(lbj_VBOVertex), (char*)NULL + 3 * sizeof(GLfloat));
	glTexCoordPointer(2, GL_FLOAT, sizeof(lbj_VBOVertex), (char*)NULL + 6 * sizeof(GLfloat));

	// Loads the default material
	lbj_LoadDefaultMaterial();
	unsigned int i, j = 0;

	// This handles the change of material
	if (model.mats.used > 0) {
		// For each face
		for (i = 0; i < model.f.used; i++) {
			if (i == 0) {
				// Load the fist material
				lbj_LoadMaterial(model.mats.array[model.matsi.array[i]]);
			}
			else if (model.matsi.array[i - 1] != model.matsi.array[i]) {
				// If the material changes draw the vertices with the current material
				// Don't draw translucent materials
				if (model.mats.array[model.matsi.array[i - 1]].Tr == 1)
					glDrawElements(GL_QUADS, (i - j) * 4, GL_UNSIGNED_INT, (char*)NULL + j * 4 * sizeof(GLint));
				j = i;
				// Load the next material
				lbj_LoadMaterial(model.mats.array[model.matsi.array[i]]);
			}
			else if (i == model.f.used - 1) {
				// If this is the last face, draw the rest of the buffer
				// Don't draw translucent materials
				if (model.mats.array[model.matsi.array[i]].Tr == 1)
					glDrawElements(GL_QUADS, (i - j + 1) * 4, GL_UNSIGNED_INT, (char*)NULL + j * 4 * sizeof(GLint));
			}
		}
	}
	else {
		// No material found. Draw the buffer normaly
		glDrawElements(GL_QUADS, model.f.used * 4, GL_UNSIGNED_INT, 0);
	}

	// Unbinds the buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Disables client states
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisable(GL_TEXTURE_2D);
}

// Whether to print stats about the model or just warnings
void lbj_PrintStats(int value)
{
	if (value) lbjp_printStats = 1;
	else lbjp_printStats = 0;
}

#endif // LOBJDER_IMPLEMENTATION

#endif // LOBJDER_H_INCLUDED