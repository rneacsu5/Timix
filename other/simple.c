
// Commands
// gcc -g simple.c -lglut -lGL -lGLU -lm -o out.x -Wall
// ./out.x

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

// Ligth parameters
GLfloat lightPos[] = {5, 5, 4, 1};
GLfloat lightAmbient[] = {0.5, 0.5, 0.5, 1};
GLfloat lightDiffuse[] = {0.8, 0.8, 0.8, 1};
GLfloat lightSpecular[] = {0.4, 0.4, 0.4, 1};

void display(void) 
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	gluLookAt(10, 10, 10, 0, 0, 0, 0, 1, 0);
	// Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	// Set the drawing color to white
	glColor3f(1, 1, 1);
	// Draw a cube
	glutSolidCube(3);
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
	gluPerspective(70, (GLdouble) width / (GLdouble) height, 0.1, 60000);
	// Load back the modelview matrix
	glMatrixMode(GL_MODELVIEW);
}

void initialize(void)
{
	// Set the background to black
	glClearColor(0, 0, 0, 1);
	// Enables depth test
	glEnable(GL_DEPTH_TEST);
	// Disable color material
	glDisable(GL_COLOR_MATERIAL);
	// Enable normalize 
	glEnable(GL_NORMALIZE);

	// Sets the light
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	// Enables lighting
	glEnable(GL_LIGHTING);
	// Enables the light
	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Test");
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	initialize();
	glutMainLoop();
	return 0;
}