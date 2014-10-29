/*
	vector library
	Basic vector support for OpenGL
	Copyright (c) 2014 Neacsu Razvan

	Use this line before you include this header to create the implementation. USE IT ONLY ONCE!
		#define VECTOR_IMPLEMENTATION
	
*/

// Include guard (if multiple files are using this file, the guard will include the text only once to avoid redefinition)
#ifndef VECTOR_H

#define VECTOR_H

#include <stdio.h>
#include <math.h>
#include <GL/glut.h>


// Vector structure
typedef struct
{
	GLdouble x, y, z;
} vect_Vector;

vect_Vector vect_Create(GLdouble x, GLdouble y, GLdouble z);

// Normalizes given vector (makes its lenght equal to 1)
void vect_Normalize(vect_Vector *v);

// Returns the vector multiplied by the factor
vect_Vector vect_Multiply(vect_Vector v, GLdouble factor);

// Returns the sum of given vectors
vect_Vector vect_Add(vect_Vector v1, vect_Vector v2);

// Returns the difference of given vectors
vect_Vector vect_Substract(vect_Vector v1, vect_Vector v2);

// Retruns the vector rotated by "angle" degrees on the given axis (x, y, z) 
vect_Vector vect_Rotate(vect_Vector v, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);

// Returns the vector's length
GLdouble vect_Length(vect_Vector v);

// Prints the vector coordonates
void vect_Print(vect_Vector v);

#ifdef VECTOR_IMPLEMENTATION

// Returns a vector with given x, y, z
vect_Vector vect_Create(GLdouble x, GLdouble y, GLdouble z)
{
	vect_Vector v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

// Normalizes given vector (makes its lenght equal to 1)
void vect_Normalize(vect_Vector *v)
{

	GLdouble modul = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
	if (modul != 0) {
		(*v).x /= modul;
		(*v).y /= modul;
		(*v).z /= modul;
	}
}

// Returns the vector multiplied by the factor
vect_Vector vect_Multiply(vect_Vector v, GLdouble factor)
{
	vect_Vector out = v;
	out.x *= factor;
	out.y *= factor;
	out.z *= factor;
	return out;
}

// Returns the sum of given vectors
vect_Vector vect_Add(vect_Vector v1, vect_Vector v2)
{
	vect_Vector sum;
	sum.x = v1.x + v2.x;
	sum.y = v1.y + v2.y;
	sum.z = v1.z + v2.z;
	return sum;
}

// Returns the difference of given vectors
vect_Vector vect_Substract(vect_Vector v1, vect_Vector v2)
{
	vect_Vector diff;
	diff.x = v1.x - v2.x;
	diff.y = v1.y - v2.y;
	diff.z = v1.z - v2.z;
	return diff;
}

// Retruns the vector rotated by "angle" degrees on the given axis (x, y, z) 
vect_Vector vect_Rotate(vect_Vector v, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	// Normalizes the rotation motVector
	vect_Vector n = vect_Create(x, y, z);
	vect_Normalize(&n);
	x = n.x;
	y = n.y;
	z = n.z;

	GLdouble r = angle * 3.14159265359 / 180.0;
	GLdouble c = cos(r);
	GLdouble s = sin(r);

	// Rotates the motVector
	vect_Vector out;
	out.x = (x * x * (1 - c) + c) * v.x + (x * y * (1 - c) - z * s) * v.y + (x * z * (1 - c) + y * s) * v.z;
	out.y = (y * x * (1 - c) + z * s) * v.x + (y * y * (1 - c) + c) * v.y + (y * z * (1 - c) - x * s) * v.z;
	out.z = (x * z * (1 - c) - y * s) * v.x + (y * z * (1 - c) + x * s) * v.y + (z * z * (1 - c) + c) * v.z;

	return out;
}

// Returns the vector's length
GLdouble vect_Length(vect_Vector v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Prints the vector coordonates
void vect_Print(vect_Vector v)
{
	printf("%lf %lf %lf\n", v.x, v.y, v.z);
}

#endif // VECTOR_IMPLEMENTATION

#endif // VECTOR_H