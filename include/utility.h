#include <GL/glut.h>
#include "bitmap.h"

#define PI 3.14159265359
#define DEG_TO_RAD PI / 180
#define RAD_TO_DEG 180 / PI

typedef struct {
	GLdouble x, y, z;
} vector;

// A new structure for textures
typedef struct {
	GLubyte* texData;     // Texture data (extracted from the .bmp file using bitmap.h)
	BITMAPINFO* texInfo;  // Texture info (extracted from the .bmp file using bitmap.h)
	GLuint texName;       // Texture name: used with glBindTexture(GL_TEXTURE_2D, texName) to swich to diffrent textures
	GLsizei texWidth;     // Texture width
	GLsizei texHeight;    // Texture height
} Texture;

void normalizev(vector *v);
void multiplyv(vector *v, GLdouble factor);
vector addv(vector v1, vector v2);
vector substractv(vector v1, vector v2);
vector rotatev(vector v, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
vector createv(GLdouble x, GLdouble y, GLdouble z);
GLdouble vlength(vector v);
void printv(vector v);

// Fuction to load a .bmp file to a texture
void loadBMP(char *fileName, Texture *tex);

// Loads two Shaders from files
void loadShaders(char * path1, GLenum type1, char * path2, GLenum type2);