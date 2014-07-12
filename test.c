#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265

int a = 0;
float r;
float loc[] = {5, 2.5, 5, 1};
float white_color[] = {0.5, 0.5, 0.5, 1};

void display(void)
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	gluLookAt(5, 1.7, 5, 5 + sin(r), 1.7, 5 + cos(r), 0, 1, 0);
	// Set camera position
	glLightfv(GL_LIGHT0, GL_POSITION, loc);
	// Set the drawing color to cyan
	glColor3f(0, 1, 1);

	glPushMatrix();
	glTranslatef(2, 0.15, 2);
	glRotatef(a, 0, 1, 0);
	glutSolidTeapot(0.3);
	glPopMatrix();

	int i, j;

	//Floor
	glColor3f(0.7, 1, 0.7);
	glBegin(GL_QUADS);
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			glVertex3f(i, 0, j);
			// glTexCoord2f(i, j);
			glNormal3f(0, 1, 0);

			glVertex3f(i, 0, j + 1);
			// glTexCoord2f(i, j + 1);
			glNormal3f(0, 1, 0);

			glVertex3f(i + 1, 0, j + 1);
			// glTexCoord2f(i + 1, j + 1);
			glNormal3f(0, 1, 0);

			glVertex3f(i + 1, 0, j);
			// glTexCoord2f(i + 1, j);
			glNormal3f(0, 1, 0);

		}
	}
	glEnd();

	//Wall 1
	glColor3f(1, 1, 0);
	glBegin(GL_QUADS);
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j ++) {
			glVertex3f(i, 0.25 * j, 0);
			// glTexCoord2f(i, k);
			glNormal3f(0, 0, 1);

			glVertex3f(i, 0.25 * (j + 1), 0);
			// glTexCoord2f(i, k + 1);
			glNormal3f(0, 0, 1);

			glVertex3f(i + 1, 0.25 * (j + 1), 0);
			// glTexCoord2f(i + 1, k + 1);
			glNormal3f(0, 0, 1);

			glVertex3f(i + 1, 0.25 * j, 0);
			// glTexCoord2f(i + 1, k);
			glNormal3f(0, 0, 1);

		}
	}
	glEnd();

	//Wall 2
	glColor3f(1, 0, 1);
	glBegin(GL_QUADS);
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j ++) {
			glVertex3f(i, 0.25 * j, 10);
			// glTexCoord2f(i, k);
			glNormal3f(0, 0, -1);

			glVertex3f(i, 0.25 * (j + 1), 10);
			// glTexCoord2f(i, k + 1);
			glNormal3f(0, 0, -1);

			glVertex3f(i + 1, 0.25 * (j + 1), 10);
			// glTexCoord2f(i + 1, k + 1);
			glNormal3f(0, 0, -1);

			glVertex3f(i + 1, 0.25 * j, 10);
			// glTexCoord2f(i + 1, k);
			glNormal3f(0, 0, -1);

		}
	}
	glEnd();

	//Wall 3
	glColor3f(0, 1, 1);
	glBegin(GL_QUADS);
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j ++) {
			glVertex3f(0, 0.25 * j, i);
			// glTexCoord2f(i, k);
			glNormal3f(1, 0, 0);

			glVertex3f(0, 0.25 * (j + 1), i);
			// glTexCoord2f(i, k + 1);
			glNormal3f(1, 0, 0);

			glVertex3f(0, 0.25 * (j + 1), i + 1);
			// glTexCoord2f(i + 1, k + 1);
			glNormal3f(1, 0, 0);

			glVertex3f(0, 0.25 * j, i + 1);
			// glTexCoord2f(i + 1, k);
			glNormal3f(1, 0, 0);

		}
	}
	glEnd();

	//Wall 4
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j ++) {
			glVertex3f(10, 0.25 * j, i);
			// glTexCoord2f(i, k);
			glNormal3f(-1, 0, 0);

			glVertex3f(10, 0.25 * (j + 1), i);
			// glTexCoord2f(i, k + 1);
			glNormal3f(-1, 0, 0);

			glVertex3f(10, 0.25 * (j + 1), i + 1);
			// glTexCoord2f(i + 1, k + 1);
			glNormal3f(-1, 0, 0);

			glVertex3f(10, 0.25 * j, i + 1);
			// glTexCoord2f(i + 1, k);
			glNormal3f(-1, 0, 0);

		}
	}
	glEnd();
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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_color);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_color);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
}
void tick(int value)
{
	a++;
	if (a == 360)
		a = 0;
	r = a * PI / 180;
	glutPostRedisplay();
	glutTimerFunc(10, tick, value);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Game");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	initialize();
	glutTimerFunc(10, tick, 0);
	glutMainLoop();
	return 0;
}
