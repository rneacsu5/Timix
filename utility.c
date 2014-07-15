#include <GL/glut.h>
#include <math.h>
#include "utility.h"

#define PI 3.14159265

void normalizev(vector *v) {

	GLfloat modul = sqrt((*v).x * (*v).x + (*v).y * (*v).y + (*v).z * (*v).z);
	(*v).x /= modul;
	(*v).y /= modul;
	(*v).z /= modul; 
}

void multiplyv(vector *v, GLfloat factor) 
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
	vector sum;
	sum.x = v1.x - v2.x;
	sum.y = v1.y - v2.y;
	sum.z = v1.z - v2.z;
	return sum;
}

vector rotatev(vector v, GLfloat a) 
{
	vector r;
	GLfloat ar = a * PI / 180;
	r.x = v.x * cos(ar) + v.z * sin(ar);
	r.y = v.y;
	r.z = -v.z * cos(ar) - v.x * sin(ar);
	return r;
}

vector createv(GLfloat x, GLfloat y, GLfloat z) 
{
	vector v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}