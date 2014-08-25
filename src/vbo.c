
// Commands
// gcc vbo.c utility.c motion.c -lglut -lGL -lGLU -lGLEW -lm -o out.x -Wall
// ./out.x

#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include "../include/utility.h"
#include "../include/motion.h"

// C does not support boolean
#define true 1
#define false 0

#define ELEMENTS 100000

// Ligth parameters
GLfloat lightPos[] = {0, 3, 1, 1};
GLfloat lightAmbient[] = {0.5, 0.5, 0.5, 1};
GLfloat lightDiffuse[] = {0.8, 0.8, 0.8, 1};
GLfloat lightSpecular[] = {0.4, 0.4, 0.4, 1};

GLuint vertexBuffID, indexBuffID; 

typedef struct {
	GLfloat pos[3];
	GLfloat color[4];
	GLfloat normal[3];
} Vertex;

Vertex vertices[4 * ELEMENTS];
GLint indices[4 * ELEMENTS];
GLint numOfVertices = 0;
GLint numOfIndices = 0;


void display(void)
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	// Set up the camera
	motMoveCamera();

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffID);

	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), NULL + 0);
	glColorPointer(4, GL_FLOAT, sizeof(Vertex), NULL + 3 * sizeof(GLfloat));
	glNormalPointer(GL_FLOAT, sizeof(Vertex), NULL + 7 * sizeof(GLfloat));

	glDrawElements(GL_QUADS, numOfIndices, GL_UNSIGNED_INT, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
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
	// Set the background to black
	glClearColor(0, 0, 0, 1);
	// Enables depth test
	glEnable(GL_DEPTH_TEST);
	// Enables color material
	glEnable(GL_COLOR_MATERIAL);
	// Enables normalize 
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

	// LAG MAKER
	// Increase ELEMENTS for more lag
	int i;
	for (i = 0; i < ELEMENTS; i++) {
		vertices[4 * i + 0].pos[0] = 0; vertices[4 * i + 0].pos[1] = 0; vertices[4 * i + 0].pos[2] = i / 10.0;
		vertices[4 * i + 1].pos[0] = 3; vertices[4 * i + 1].pos[1] = 0; vertices[4 * i + 1].pos[2] = i / 10.0;
		vertices[4 * i + 2].pos[0] = 3; vertices[4 * i + 2].pos[1] = 3; vertices[4 * i + 2].pos[2] = i / 10.0;
		vertices[4 * i + 3].pos[0] = 0; vertices[4 * i + 3].pos[1] = 3; vertices[4 * i + 3].pos[2] = i / 10.0;

		vertices[4 * i + 0].color[0] = 0; vertices[4 * i + 0].color[1] = 1; vertices[4 * i + 0].color[2] = 1; vertices[4 * i + 0].color[3] = 1;
		vertices[4 * i + 1].color[0] = 0; vertices[4 * i + 1].color[1] = 0; vertices[4 * i + 1].color[2] = 1; vertices[4 * i + 1].color[3] = 1;
		vertices[4 * i + 2].color[0] = 1; vertices[4 * i + 2].color[1] = 0; vertices[4 * i + 2].color[2] = 1; vertices[4 * i + 2].color[3] = 1;
		vertices[4 * i + 3].color[0] = 0; vertices[4 * i + 3].color[1] = 1; vertices[4 * i + 3].color[2] = 0; vertices[4 * i + 3].color[3] = 1;

		vertices[4 * i + 0].normal[0] = 0; vertices[4 * i + 0].normal[1] = 0; vertices[4 * i + 0].normal[2] = 1; 
		vertices[4 * i + 1].normal[0] = 0; vertices[4 * i + 1].normal[1] = 0; vertices[4 * i + 1].normal[2] = 1; 
		vertices[4 * i + 2].normal[0] = 0; vertices[4 * i + 2].normal[1] = 0; vertices[4 * i + 2].normal[2] = 1; 
		vertices[4 * i + 3].normal[0] = 0; vertices[4 * i + 3].normal[1] = 0; vertices[4 * i + 3].normal[2] = 1; 

		indices[4 * i + 0] = 4 * i + 0;
		indices[4 * i + 1] = 4 * i + 1;
		indices[4 * i + 2] = 4 * i + 2;
		indices[4 * i + 3] = 4 * i + 3;

		numOfVertices += 4;
		numOfIndices += 4;
	}

	glGenBuffers(1, &vertexBuffID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffID);
	glBufferData(GL_ARRAY_BUFFER, numOfVertices * sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &indexBuffID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numOfIndices * sizeof(GLint), indices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void tick (int value) {
	glutPostRedisplay();
	glutTimerFunc(10, tick, 0);
}

int main(int argc, char *argv[])
{
	printf("===================================================\n");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("VBO Test");
	motionInit();

	// Event listeners
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	
	initialize();

	glutTimerFunc(10, tick, 0);

	glutMainLoop();
	return 0;
}