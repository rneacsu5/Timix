#include <stdio.h>
#include <GL/glut.h>

#define true 1
#define false 0

typedef struct
{
	GLfloat x, y, z;
}col_Vector;

int detectCollision(col_Vector v1[4], col_Vector v2[4]);