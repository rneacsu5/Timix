#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "utility.h"

// C does not support boolean?! WTF!
#define true 7
#define false 0
#define EYE_HEIGHT 1.7
#define ACCELERATION 0.02
#define DRAG 0.01
#define MAX_SPEED 0.15
// The formula is: GFORCE = 2 * heightOfJump / ((timeOfJump / 2) * (timeOfJump / 2 + 1))   where heigthOfJump is in meters and timeOfJump is in milliseconds
#define GFORCE 0.0041666666

float a1 = 0, a2 = 0, r;
float lightPos[] = {5, 2.4, 5, 1};
float lightColor[] = {0.5, 0.5, 0.5, 1};
int viewHeight, viewWidth;
GLfloat cameraXAngle = 225, cameraYAngle = 0;
vector eye, target, cameraForce, gForce;
int test = 0;
int keyStates[256];

void drawWallsAndFloor(void)
{
	GLdouble i, j;
	// The walls and floor will be a (10 / nr) * (10 / nr) grid
	GLfloat nr = 0.1;
	glBegin(GL_QUADS);
		for (i = 0; i < 10; i += nr) {
			for (j = 0; j < 10; j += nr) {
				// Floor
				glColor3f(i / 10.0, 0, j / 10.0);
				glNormal3f(0, 1, 0);
				glVertex3f(i, 0, j);
				// glTexCoord2f(i, j);
				glVertex3f(i, 0, j + nr);
				// glTexCoord2f(i, j + 1);
				glVertex3f(i + nr, 0, j + nr);
				// glTexCoord2f(i + 1, j + 1);
				glVertex3f(i + nr, 0, j);
				// glTexCoord2f(i + 1, j);

				// Wall 1
				glColor3f(j / 10.0, 0, i / 10.0);
				glNormal3f(0, 0, 1);
				glVertex3f(i, 0.25 * j, 0);
				// glTexCoord2f(i, k);
				glVertex3f(i, 0.25 * (j + nr), 0);
				// glTexCoord2f(i, k + 1);
				glVertex3f(i + nr, 0.25 * (j + nr), 0);
				// glTexCoord2f(i + 1, k + 1);
				glVertex3f(i + nr, 0.25 * j, 0);
				// glTexCoord2f(i + 1, k);

				// Wall 2
				glColor3f(j / 10.0, i / 10.0, 0);
				glNormal3f(0, 0, -1);
				glVertex3f(i, 0.25 * j, 10);
				// glTexCoord2f(i, k);
				glVertex3f(i, 0.25 * (j + nr), 10);
				// glTexCoord2f(i, k + 1);
				glVertex3f(i + nr, 0.25 * (j + nr), 10);
				// glTexCoord2f(i + 1, k + 1);
				glVertex3f(i + nr, 0.25 * j, 10);
				// glTexCoord2f(i + 1, k);

				// Wall 3
				glColor3f(0, i / 10.0, j / 10.0);
				glNormal3f(1, 0, 0);
				glVertex3f(0, 0.25 * j, i);
				// glTexCoord2f(i, k);
				glVertex3f(0, 0.25 * (j + nr), i);
				// glTexCoord2f(i, k + 1);
				glVertex3f(0, 0.25 * (j + nr), i + nr);
				// glTexCoord2f(i + 1, k + 1);
				glVertex3f(0, 0.25 * j, i + nr);
				// glTexCoord2f(i + 1, k);

				// Wall 4
				glColor3f(j / 10.0, 1, i / 10.0);
				glNormal3f(-1, 0, 0);
				glVertex3f(10, 0.25 * j, i);
				// glTexCoord2f(i, k);
				glVertex3f(10, 0.25 * (j + nr), i);
				// glTexCoord2f(i, k + 1);
				glVertex3f(10, 0.25 * (j + nr), i + nr);
				// glTexCoord2f(i + 1, k + 1);
				glVertex3f(10, 0.25 * j, i + nr);
				// glTexCoord2f(i + 1, k);

				// Ceiling
				glColor3f(i / 10.0, 0, j / 10.0);
				glNormal3f(0, -1, 0);
				glVertex3f(i, 2.5, j);
				// glTexCoord2f(i, j);
				glVertex3f(i, 2.5, j + nr);
				// glTexCoord2f(i, j + 1);
				glVertex3f(i + nr, 2.5, j + nr);
				// glTexCoord2f(i + 1, j + 1);
				glVertex3f(i + nr, 2.5, j);
				// glTexCoord2f(i + 1, j);
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

void moveCamera(void) 
{
	// Aplys drag to the camera force if he isn't jumping
	if (keyStates[32] == false) {
		vector drag = cameraForce;
		normalizev(&drag);
		if (vlength(cameraForce) - DRAG > 0) {
			multiplyv(&drag, vlength(cameraForce) - DRAG);
		}
		else {
			drag = createv(0, 0, 0);
		}
		cameraForce = drag;
	}

	// Makes the acceleration vector based on input and direction
	vector direction = substractv(createv(target.x, EYE_HEIGHT, target.z), eye);
	normalizev(&direction);
	vector acc = createv(0, 0, 0);
	if (keyStates['w']) {
		acc = addv(acc, direction);
	}
	if (keyStates['s']) {
		acc = substractv(acc, direction);
	}
	if (keyStates['a']) {
		acc = addv(acc, rotatev(direction, 90, 0, 1, 0));
	}
	if (keyStates['d']) {
		acc = addv(acc, rotatev(direction, -90, 0, 1, 0));
	}

	// Makes the acceleration vector the right length
	normalizev(&acc);
	multiplyv(&acc, ACCELERATION);

	// Adds the acceleration vector to the camera force if he isn't jumping
	if (keyStates[32] == false) {
		cameraForce = addv(cameraForce, acc);
	}

	// Gravity
	cameraForce = addv(cameraForce, gForce);

	// Makes sure camera force does not exceed MAX_SPEED (again if he isn't jumping)
	if (vlength(cameraForce) > MAX_SPEED && keyStates[32] == false) {
		normalizev(&cameraForce);
		multiplyv(&cameraForce, MAX_SPEED);
	}

	// Moves eye and target
	eye = addv(eye, cameraForce);
	target = addv(target, cameraForce);

	// Check for floor
	if (eye.y <= EYE_HEIGHT) {
		target.y += EYE_HEIGHT - eye.y;
		eye.y = EYE_HEIGHT;
		cameraForce = createv(cameraForce.x, 0, cameraForce.z);
		keyStates[32] = false;
	}
}

void jumpFunc(void) {
	if (keyStates[32] == false) {
		keyStates[32] = true;
		gForce = createv(0, - GFORCE, 0);
		//                                  GFORCE * (timeOfJump / 2 + 1)
		//                                               ||
		cameraForce = addv(cameraForce, createv(0, GFORCE * 16, 0));
	}
}

void display(void)
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	moveCamera();
	gluLookAt(eye.x, eye.y, eye.z, target.x, target.y, target.z, 0, 1, 0);
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

	viewHeight = height;
	viewWidth = width;
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
	// Represents the camera position
	eye = createv(5, EYE_HEIGHT, 5);
	// Represents the point where the camera looks
	target = createv(0, EYE_HEIGHT, 0);
	// A vector that moves the camera
	cameraForce = createv(0, 0, 0);
	// Gravity
	gForce = createv(0, 0, 0);

}

void freeCameraHandler (int x, int y) {
	// Determines the angle on each axis based on mouse position
	cameraXAngle += -45 + 90 * x / (double) viewWidth;
	cameraYAngle += -30 + 60 * y / (double) viewHeight;

	// cameraXAngle must not exeed 360 or be below -360 degrees
	cameraXAngle -= ((int) cameraYAngle / 360) * 360;

	// cameraYAngle must not exeed 90 degrees or be below -90 degrees
	if (cameraYAngle >= 89) 
		cameraYAngle = 89;
	if (cameraYAngle <= -89) 
		cameraYAngle = -89;

	// Some notations
	GLfloat sinY = sin(-cameraYAngle * DEG_TO_RAD);
	GLfloat sinX = sin(cameraXAngle * DEG_TO_RAD);
	GLfloat cosX = cos(cameraXAngle * DEG_TO_RAD);

	// Makes the target vector based on cameraXAngle and cameraYAngle (rotating the vector using rotatev() was a bit buggy)
	target = createv(cosX, 0, sinX);
	multiplyv(&target, sqrt(1 - sinY * sinY));
	target = addv(target, createv(0, sinY, 0));

	// Adds the eye position so that the camera points to the right place
	target = addv(target, eye);	
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

	// Moves mouse to the middle
	glutWarpPointer(viewWidth / 2, viewHeight / 2);
	
	// Calls the display() function
	glutPostRedisplay();

	// Waits 10 ms
	glutTimerFunc(10, tick, value + 1);
}

// Handles normal key presses (arrow, function and other keys not included)
void normalKeysHandler(unsigned char key, int x, int y) 
{
	switch(key) {
		case 'W':
		case 'w':
			keyStates['w'] = true;
			break;
		case 'S':
		case 's':
			keyStates['s'] = true;
			break;
		case 'A':
		case 'a':
			keyStates['a'] = true;
			break;
		case 'D':
		case 'd':
			keyStates['d'] = true;
			break;
		// Space Key
		case 32:
			jumpFunc();
			break;
		// Esc Key
		case 27:
			exit(0);
			break;
	}
}

// Handles arrow, function and other keys presses (and maps them with normal keys)
void specialKeysHandler(int key, int x, int y) 
{
	switch(key) {
		case GLUT_KEY_UP:
			keyStates['w'] = true;
			break;
		case GLUT_KEY_DOWN:
			keyStates['s'] = true;
			break;
		case GLUT_KEY_LEFT:
			keyStates['a'] = true;
			break;
		case GLUT_KEY_RIGHT:
			keyStates['d'] = true;
			break;

	}
}

// Handles normal keys releases (arrow, function and other keys not included)
void normalKeysUpHandler (unsigned char key, int x, int y) 
{
	switch(key) {
		case 'W':
		case 'w':
			keyStates['w'] = false;
			break;
		case 'S':
		case 's':
			keyStates['s'] = false;
			break;
		case 'A':
		case 'a':
			keyStates['a'] = false;
			break;
		case 'D':
		case 'd':
			keyStates['d'] = false;
			break;
	}
}

// Handles arrow, function and other keys realeses (and maps them with normal keys releases)
void specialKeysUpHandler(int key, int x, int y) 
{
	switch(key) {
		case GLUT_KEY_UP:
			keyStates['w'] = false;
			break;
		case GLUT_KEY_DOWN:
			keyStates['s'] = false;
			break;
		case GLUT_KEY_LEFT:
			keyStates['a'] = false;
			break;
		case GLUT_KEY_RIGHT:
			keyStates['d'] = false;
			break;
	}
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Epic Game");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(normalKeysHandler);
	glutKeyboardUpFunc(normalKeysUpHandler);
	glutSpecialFunc(specialKeysHandler);
	glutSpecialUpFunc(specialKeysUpHandler);
	glutPassiveMotionFunc(freeCameraHandler);
	glutMotionFunc(freeCameraHandler);
	initialize();
	// Starts main timer
	glutTimerFunc(10, tick, 0);
	glutMainLoop();
	return 0;
}
