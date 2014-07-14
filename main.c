#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265

float a1 = 0;
float a2 = 0;
float r;
float lightPos[] = {5, 2.5, 5, 1};
float lightColor[] = {0.5, 0.5, 0.5, 1};

void drawWallsAndFloor(void)
{
	int i, j;
	glBegin(GL_QUADS);
		for (i = 0; i < 10; i++) {
			for (j = 0; j < 10; j++) {
				// Floor
				glColor3f(i / 10.0, 0, j / 10.0);
				glNormal3f(0, 1, 0);
				glVertex3f(i, 0, j);
				// glTexCoord2f(i, j);
				glVertex3f(i, 0, j + 1);
				// glTexCoord2f(i, j + 1);
				glVertex3f(i + 1, 0, j + 1);
				// glTexCoord2f(i + 1, j + 1);
				glVertex3f(i + 1, 0, j);
				// glTexCoord2f(i + 1, j);

				// Wall 1
				glColor3f(j / 10.0, 0, i / 10.0);
				glNormal3f(0, 0, 1);
				glVertex3f(i, 0.25 * j, 0);
				// glTexCoord2f(i, k);
				glVertex3f(i, 0.25 * (j + 1), 0);
				// glTexCoord2f(i, k + 1);
				glVertex3f(i + 1, 0.25 * (j + 1), 0);
				// glTexCoord2f(i + 1, k + 1);
				glVertex3f(i + 1, 0.25 * j, 0);
				// glTexCoord2f(i + 1, k);

				// Wall 2
				glColor3f(j / 10.0, i / 10.0, 0);
				glNormal3f(0, 0, -1);
				glVertex3f(i, 0.25 * j, 10);
				// glTexCoord2f(i, k);
				glVertex3f(i, 0.25 * (j + 1), 10);
				// glTexCoord2f(i, k + 1);
				glVertex3f(i + 1, 0.25 * (j + 1), 10);
				// glTexCoord2f(i + 1, k + 1);
				glVertex3f(i + 1, 0.25 * j, 10);
				// glTexCoord2f(i + 1, k);

				// Wall 3
				glColor3f(0, i / 10.0, j / 10.0);
				glNormal3f(1, 0, 0);
				glVertex3f(0, 0.25 * j, i);
				// glTexCoord2f(i, k);
				glVertex3f(0, 0.25 * (j + 1), i);
				// glTexCoord2f(i, k + 1);
				glVertex3f(0, 0.25 * (j + 1), i + 1);
				// glTexCoord2f(i + 1, k + 1);
				glVertex3f(0, 0.25 * j, i + 1);
				// glTexCoord2f(i + 1, k);

				// Wall 4
				glColor3f(j / 10.0, 1, i / 10.0);
				glNormal3f(-1, 0, 0);
				glVertex3f(10, 0.25 * j, i);
				// glTexCoord2f(i, k);
				glVertex3f(10, 0.25 * (j + 1), i);
				// glTexCoord2f(i, k + 1);
				glVertex3f(10, 0.25 * (j + 1), i + 1);
				// glTexCoord2f(i + 1, k + 1);
				glVertex3f(10, 0.25 * j, i + 1);
				// glTexCoord2f(i + 1, k);
			}
		}
	glEnd();
}

void drawCube(float size) 
{
	int i, j;
	float s = size / 10;
	glPushMatrix();
		glTranslatef(-size / 2, -size / 2, -size / 2);
		glBegin(GL_QUADS);
			for (i = 0; i < 10; i++) {
				for (j = 0; j < 10; j++) {
					glNormal3f(0, -1, 0);
					glVertex3f(s * i, 0 , s * j);
					glVertex3f(s * (i + 1), 0 , s * j);
					glVertex3f(s * (i + 1) , 0 , s * (j + 1));
					glVertex3f(s * i, 0 , s * (j + 1));

					glNormal3f(0, 0, -1);
					glVertex3f(s * i, s * j, 0);
					glVertex3f(s * (i + 1), s * j, 0);
					glVertex3f(s * (i + 1) , s * (j + 1), 0);
					glVertex3f(s * i, s * (j + 1), 0);

					glNormal3f(1, 0, 0);
					glVertex3f(size, s * i, s * j);
					glVertex3f(size, s * i, s * (j + 1));
					glVertex3f(size, s * (i + 1), s * (j + 1));
					glVertex3f(size, s * (i + 1), s * j);

					glNormal3f(0, 0, 1);
					glVertex3f(s * i, s * j, size);
					glVertex3f(s * i, s * (j + 1), size);
					glVertex3f(s * (i + 1), s * (j + 1), size);
					glVertex3f(s * (i + 1), s * j, size);

					glNormal3f(-1, 0, 0);
					glVertex3f(0, s * i, s * j);
					glVertex3f(0, s * i, s * (j + 1));
					glVertex3f(0, s * (i + 1), s * (j + 1));
					glVertex3f(0, s * (i + 1), s * j);

					glNormal3f(0, 1, 0);
					glVertex3f(s * i, size , s * j);
					glVertex3f(s * (i + 1), size , s * j);
					glVertex3f(s * (i + 1) , size , s * (j + 1));
					glVertex3f(s * i, size , s * (j + 1));
				}
			}
		glEnd();
	glPopMatrix();
}

void display(void)
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	// Note: Delete "* 0" for a tilted camera
	gluLookAt(5, 1.7, 5, 5 + sin(r), 1.7, 5 + cos(r), -cos(r) * 0, 3, sin(r) * 0);
	// Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	// Draws and rotates a cyan teapot
	glPushMatrix();
		glTranslatef(2, 0.15, 2);
		glRotatef(a2, 0, 1, 0);
		glColor3f(0, 1, 1);
		glutSolidTeapot(0.3);
	glPopMatrix();

	drawWallsAndFloor();

	// Draws and animates a green cube
	glPushMatrix();
		float k = (a2 - ((int) a2 / 90) * 90) * 2 * PI / 180;
		glTranslatef(5 + 4 * sin(r), 0.5 + sin(k) * (sqrt(2) / 2 - 0.5), 5 + 4 * cos(r));
		glRotatef(a2, -sin(r), 0, -cos(r));
		glRotatef(a1, 0, 1, 0);
		glColor3f(0,0.5,0);
		drawCube(1);
	glPopMatrix();

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
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
}
void tick(int value)
{
	a1+= 5 * 0.1;
	if (a1 >= 360)
		a1 = 0;
	r = a1 * PI / 180;

	a2 += 5 * 0.62831;
	if (a2 >= 360)
		a2 = 0;

	glutPostRedisplay();
	glutTimerFunc(10, tick, value);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("Epic Game");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	initialize();
	glutTimerFunc(10, tick, 0);
	glutMainLoop();
	return 0;
}
