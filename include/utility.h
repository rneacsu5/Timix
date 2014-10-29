#include <GL/glew.h>

#define PI 3.14159265359
#define DEG_TO_RAD PI / 180
#define RAD_TO_DEG 180 / PI

// Loads two Shaders from files
void loadShaders(char * path1, GLenum type1, char * path2, GLenum type2);