#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265

int a = 0;
float r;
float loc[] = {5, 2.5, 5, 1};
float color[] = {1, 1, 1, 1};


void display(void) 
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	gluLookAt(5, 1.7, 5, 5 + sin(r), 1.7, 5 + cos(r), 0, 1, 0);

	glLightfv(GL_LIGHT0, GL_POSITION, loc);
	// Set the drawing color to white
	glColor3f(0, 1, 1);
	// Draw a wireframe teapot of size 5
	glPushMatrix();
		glTranslatef(2, 0.1, 2);
		glRotatef(a, 0, 1, 0);
		glutSolidTeapot(0.2);
	glPopMatrix();

	//Floor
	glBegin(GL_QUADS);
		int i, j;
		for (i = 0; i < 5; i++) {
			for (j = 0; j < 5; j++) {
				glVertex3f(2 * i, 0, 2 * j);
				// glTexCoord2f(i, j);
				glNormal3f(0, 1, 0);

				glVertex3f(2 * i, 0, 2 * j + 2);
				// glTexCoord2f(i, j + 1);
				glNormal3f(0, 1, 0);

				glVertex3f(2 * i + 2, 0, 2 * j + 2);
				// glTexCoord2f(i + 1, j + 1);
				glNormal3f(0, 1, 0);

				glVertex3f(2 * i + 2, 0, 2 * j);
				// glTexCoord2f(i + 1, j);
				glNormal3f(0, 1, 0);

			}
		}
	glEnd();

	float k;

	//Wall 1
	glBegin(GL_QUADS);
		for (i = 0; i < 5; i++) {
			for (k = 0; k < 1.25; k += 0.25) {
				glVertex3f(2 * i, 2 * k, 0);
				// glTexCoord2f(i, k);
				glNormal3f(0, 0, 1);

				glVertex3f(2 * i, 2 * k + 2, 0);
				// glTexCoord2f(i, k + 1);
				glNormal3f(0, 0, 1);

				glVertex3f(2 * i + 2, 2 * k + 2, 0);
				// glTexCoord2f(i + 1, k + 1);
				glNormal3f(0, 0, 1);

				glVertex3f(2 * i + 2, 2 * k, 0);
				// glTexCoord2f(i + 1, k);
				glNormal3f(0, 0, 1);

			}
		}
	glEnd();

	//Wall 2
	glBegin(GL_QUADS);
		for (i = 0; i < 5; i++) {
			for (k = 0; k < 1.25; k += 0.25) {
				glVertex3f(2 * i, 2 * k, 10);
				// glTexCoord2f(i, k);
				glNormal3f(0, 0, -1);

				glVertex3f(2 * i, 2 * k + 2, 10);
				// glTexCoord2f(i, k + 1);
				glNormal3f(0, 0, -1);

				glVertex3f(2 * i + 2, 2 * k + 2, 10);
				// glTexCoord2f(i + 1, k + 1);
				glNormal3f(0, 0, -1);

				glVertex3f(2 * i + 2, 2 * k, 10);
				// glTexCoord2f(i + 1, k);
				glNormal3f(0, 0, -1);

			}
		}
	glEnd();

	//Wall 3
	glBegin(GL_QUADS);
		for (i = 0; i < 5; i++) {
			for (k = 0; k < 1.25; k += 0.25) {
				glVertex3f(0, 2 * k, 2 * i);
				// glTexCoord2f(i, k);
				glNormal3f(1, 0, 0);

				glVertex3f(0, 2 * k + 2, 2 * i);
				// glTexCoord2f(i, k + 1);
				glNormal3f(1, 0, 0);

				glVertex3f(0, 2 * k + 2, 2 * i + 2);
				// glTexCoord2f(i + 1, k + 1);
				glNormal3f(1, 0, 0);

				glVertex3f(0, 2 * k, 2 * i + 2);
				// glTexCoord2f(i + 1, k);
				glNormal3f(1, 0, 0);

			}
		}
	glEnd();

	//Wall 3
	glBegin(GL_QUADS);
		for (i = 0; i < 5; i++) {
			for (k = 0; k < 1.25; k += 0.25) {
				glVertex3f(10, 2 * k, 2 * i);
				// glTexCoord2f(i, k);
				glNormal3f(-1, 0, 0);

				glVertex3f(10, 2 * k + 2, 2 * i);
				// glTexCoord2f(i, k + 1);
				glNormal3f(-1, 0, 0);

				glVertex3f(10, 2 * k + 2, 2 * i + 2);
				// glTexCoord2f(i + 1, k + 1);
				glNormal3f(-1, 0, 0);

				glVertex3f(10, 2 * k, 2 * i + 2);
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
	gluPerspective(80, (GLdouble) width / (GLdouble) height, 0.1, 60000);
	// Load back the modelview matrix
	glMatrixMode(GL_MODELVIEW);
}

void initialize(void)
{
	// Set the background to black
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
	glLightfv(GL_LIGHT0, GL_SPECULAR, color);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
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
