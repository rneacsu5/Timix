#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include "utility.h"

void normalizev(vector *v) {

	GLdouble modul = sqrt((*v).x * (*v).x + (*v).y * (*v).y + (*v).z * (*v).z);
	if (modul != 0) {
		(*v).x /= modul;
		(*v).y /= modul;
		(*v).z /= modul; 
	}
}

void multiplyv(vector *v, GLdouble factor) 
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
	vector diff;
	diff.x = v1.x - v2.x;
	diff.y = v1.y - v2.y;
	diff.z = v1.z - v2.z;
	return diff;
}

vector rotatev(vector v, GLdouble angle, GLdouble x, GLdouble y, GLdouble z) 
{
	// Normalizes the rotation vector
	vector n = createv(x, y, z);
	normalizev(&n);
	x = n.x;
	y = n.y;
	z = n.z;

	GLdouble r = angle * DEG_TO_RAD;
	GLdouble c = cos(r);
	GLdouble s = sin(r);

	// Rotates the vector
	vector out;
	out.x = (x * x * (1 - c) + c) * v.x + (x * y * (1 - c) - z * s) * v.y + (x * z * (1 - c) + y * s) * v.z;
	out.y = (y * x * (1 - c) + z * s) * v.x + (y * y * (1 - c) + c) * v.y + (y * z * (1 - c) - x * s) * v.z;
	out.z = (x * z * (1 - c) - y * s) * v.x + (y * z * (1 - c) + x * s) * v.y + (z * z * (1 - c) + c) * v.z;

	return out;
}

vector createv(GLdouble x, GLdouble y, GLdouble z) 
{
	vector v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

GLdouble vlength(vector v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

void printv(vector v) {
	printf("%f %f %f\n", v.x, v.y, v.z);
}