#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/glext.h>

// C does not support boolean
#define true 1
#define false 0

// Ligth parameters
GLfloat lightPos[] = {10, 4.5, 10, 1};
GLfloat lightAmbient[] = {0.5, 0.5, 0.5, 1};
GLfloat lightDiffuse[] = {0.8, 0.8, 0.8, 1};
GLfloat lightSpecular[] = {0.4, 0.4, 0.4, 1};

GLuint vertexBuffID, indexBuffID; 

GLfloat vertices[] = {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1};
GLint indices[] = {0, 1, 5, 0, 5, 4, 2, 3, 7, 2, 7, 6, 2, 0, 4, 2, 4, 6, 2, 0, 1, 2, 1, 3, 1, 5, 7, 1, 7, 3, 5, 4, 6, 5, 6, 7};

void display(void)
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	// Set up the camera
	gluLookAt(5, 5, 5, 0, 0, 0, 0, 1, 0);

	glColor3f(0, 0, 0);

	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffID);

	glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), (char *) NULL + 0);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, indices);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);

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
	// Set the background to light gray
	glClearColor(0.8, 0.8, 0.8, 1);
	// Enables depth test
	glEnable(GL_DEPTH_TEST);
	// Disable color material
	glDisable(GL_COLOR_MATERIAL);
	// Enable normalize 
	glEnable(GL_NORMALIZE);
	// Enable Blend and transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Sets the light
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	// Enables lighting
	glEnable(GL_LIGHTING);
	// Enables the light
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);


	glGenBuffers(1, &vertexBuffID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &indexBuffID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


}

int main(int argc, char *argv[])
{
	printf("===================================================\n");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Epic Game");

	// Event listeners
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	
	initialize();

	glutMainLoop();
	return 0;
}