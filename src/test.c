#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <stdlib.h>

// Ligth parameters
GLfloat lightPos[] = {10, 4.5, 10, 1};
GLfloat lightAmbient[] = {0.5, 0.5, 0.5, 1};
GLfloat lightDiffuse[] = {0.8, 0.8, 0.8, 1};
GLfloat lightSpecular[] = {0.4, 0.4, 0.4, 1};
GLuint programID;

GLuint makeMeAShader(char * path, GLenum type) {
	GLuint id = glCreateShader(type);
	FILE * fp = fopen(path, "rb");
	if (fp == NULL) {
		printf("Failed to open %s. Aborting.\n", path);
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	int var = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char * code;
	code = (char *) malloc(var * sizeof(char));
	fread(code, var * sizeof(char), 1, fp);
	fclose(fp);

	glShaderSource(id, 1, (const GLchar * const *) &code, NULL);
	glCompileShader(id);
	return id;
}

GLuint makeMeAProgram(GLuint id1, GLuint id2) {
	GLuint id = glCreateProgram();
	glAttachShader(id, id1);
	glAttachShader(id, id2);
	glLinkProgram(id);
	return id;
}

void display(void)
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	gluLookAt(5, 5, 5, 0, 0, 0, 0, 1, 0);
	// Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	// Draws a teapot
	glutSolidTeapot(1);
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

void initialize(void) {
	// Set the background to light gray
	glClearColor(0.8, 0.8, 0.8, 1);
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
	// Sets the material ligthing
	glShadeModel(GL_SMOOTH);
	glutPostRedisplay();

	programID = makeMeAProgram(makeMeAShader("vshader.vsh", GL_VERTEX_SHADER), 
								makeMeAShader("fshader.fsh", GL_FRAGMENT_SHADER));
	glUseProgram(programID);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Epic Game");
	glewInit();
	// Event listeners
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	
	initialize();

	// Hides the mouse cursor
	glutSetCursor(GLUT_CURSOR_NONE);



	glutMainLoop();
	return 0;
}