#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265

int a = 0;

void display(void) 
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	gluLookAt(10, 10, 10, 0, 0, 0, 0, 1, 0);
	// Set the drawing color to white
	glColor3f(0, 1, 1);
	// Draw a wireframe teapot of size 5
	glutWireTeapot(5);
	// Swap buffers in GPU
	glutSwapBuffers();
}

void reshape(int width, int height)
{
	// Set the viewport to the full window size
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	// Load the projection matrix
	glMatrixMode(GL_PROJECTION);
	// Clear all the transformations on the projection matrix
	glLoadIdentity();
	// Set the perspective according to the window size
	gluPerspective(60, (GLdouble) width / (GLdouble) height, 0.1, 60000);
	// Load back the modelview matrix
	glMatrixMode(GL_MODELVIEW);
}

void initialize(void)
{
	// Set the background to black
	glClearColor(0, 0, 0, 1);
}
void tick(int value)
{
	a++;
	if (a == 360) 
		a = 0;
	float r = a * PI / 180;

	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	gluLookAt(15*cos(r), 10, 15*sin(r), 0, 0, 0, 0, 1, 0);
	// Set the drawing color to cyan
	glColor3f(0, 1, 1);
	// Draw a wireframe teapot of size 5
	glutWireTeapot(5);
	// Swap buffers in GPU
	glutSwapBuffers();

	glutTimerFunc(10,tick,value);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Game");
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	initialize();
	glutTimerFunc(10,tick,0);
	glutMainLoop();
	return 0;
}
