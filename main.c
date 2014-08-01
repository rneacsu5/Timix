#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "utility.h"
#include "bitmap.h"

// C does not support boolean?! WTF!
#define true 7
#define false 0
#define EYE_HEIGHT 1.7
#define ACCELERATION 0.02
#define DRAG 0.01
#define AIR_DRAG 0.0025
#define MAX_SPEED 0.10
#define SPRINT_BOOST 0.07
// The formula is: GFORCE = 2 * heightOfJump / ((timeOfJump / 2) * (timeOfJump / 2 + 1))   where heigthOfJump is in meters and timeOfJump is in milliseconds
#define GFORCE 0.00583333

GLdouble a1 = 0, a2 = 0, r;
GLfloat lightPos[] = {5, 2.4, 5, 1};
GLfloat lightColor[] = {0.5, 0.5, 0.5, 1};
GLfloat matSpecular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat matShininess[] = { 50.0 };
GLfloat matSurface[] = { 1.0, 1.0, 1.0, 0.0 };
int viewHeight, viewWidth;
GLdouble cameraXAngle = 225, cameraYAngle = 0;
vector eye, target, cameraForce, gForce;
int test = 0;
int keyStates[256];
int isJumping = false;
int isSprinting = false;
GLubyte* textureData;
BITMAPINFO* textureInfo;
GLubyte* textureData2;
BITMAPINFO* textureInfo2;

void drawWallsAndFloor(void)
{

	glMaterialfv(GL_FRONT, GL_SPECULAR,  matSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
	glMaterialfv(GL_FRONT, GL_AMBIENT,   matSurface);

	glDisable(GL_COLOR_MATERIAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, textureInfo->bmiHeader.biWidth,
			textureInfo->bmiHeader.biHeight, 0, GL_RGB,
			GL_UNSIGNED_BYTE, textureData);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);


	GLdouble i, j;
	// The walls and floor will be a (10 / nr) * (10 / nr) grid
	GLdouble nr = 0.1;
	glBegin(GL_QUADS);
		for (i = 0; i < 9.99; i += nr) {
			for (j = 0; j < 9.99; j += nr) {
				// Floor
				//glColor3f(i / 10.0, 0, j / 10.0);
				glNormal3f(0, 1, 0);
				glTexCoord2f(0.5 * i, 0.5 * j);
				glVertex3f(i, 0, j);
				glTexCoord2f(0.5 * i, 0.5 * (j + nr));
				glVertex3f(i, 0, j + nr);
				glTexCoord2f(0.5 * (i + nr), 0.5 * (j + nr));
				glVertex3f(i + nr, 0, j + nr);
				glTexCoord2f(0.5 * (i + nr), 0.5 * j);
				glVertex3f(i + nr, 0, j);
			}
		}

	glEnd();

	glMaterialfv(GL_FRONT, GL_SPECULAR,  matSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
	glMaterialfv(GL_FRONT, GL_AMBIENT,   matSurface);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, textureInfo2->bmiHeader.biWidth,
			textureInfo2->bmiHeader.biHeight, 0, GL_RGB,
			GL_UNSIGNED_BYTE, textureData2);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	
	glBegin(GL_QUADS);

		for (i = 0; i < 9.99; i += nr) {
			for (j = 0; j < 9.99; j += nr) {
				// Wall 1
				//glColor3f(j / 10.0, 0, i / 10.0);
				glNormal3f(0, 0, 1);
				glTexCoord2f(0.5 * i, 0.2 * j);
				glVertex3f(i, 0.25 * j, 0);
				glTexCoord2f(0.5 * i, 0.2 * (j + nr));
				glVertex3f(i, 0.25 * (j + nr), 0);
				glTexCoord2f(0.5 * (i + nr), 0.2 * (j + nr));
				glVertex3f(i + nr, 0.25 * (j + nr), 0);
				glTexCoord2f(0.5 * (i + nr), 0.2 * j);
				glVertex3f(i + nr, 0.25 * j, 0);

				// Wall 2
				//glColor3f(j / 10.0, i / 10.0, 0);
				glNormal3f(0, 0, -1);
				glTexCoord2f(0.5 * i, 0.2 * j);
				glVertex3f(i, 0.25 * j, 10);
				glTexCoord2f(0.5 * i, 0.2 * (j + nr));
				glVertex3f(i, 0.25 * (j + nr), 10);
				glTexCoord2f(0.5 * (i + nr), 0.2 * (j + nr));
				glVertex3f(i + nr, 0.25 * (j + nr), 10);
				glTexCoord2f(0.5 * (i + nr), 0.2 * j);
				glVertex3f(i + nr, 0.25 * j, 10);

				// Wall 3
				//glColor3f(0, i / 10.0, j / 10.0);
				glNormal3f(1, 0, 0);
				glTexCoord2f(0.5 * i, 0.2 * j);
				glVertex3f(0, 0.25 * j, i);
				glTexCoord2f(0.5 * i, 0.2 * (j + nr));
				glVertex3f(0, 0.25 * (j + nr), i);
				glTexCoord2f(0.5 * (i + nr), 0.2 * (j + nr));
				glVertex3f(0, 0.25 * (j + nr), i + nr);
				glTexCoord2f(0.5 * (i + nr), 0.2 * j);
				glVertex3f(0, 0.25 * j, i + nr);

				// Wall 4
				//glColor3f(j / 10.0, 1, i / 10.0);
				glNormal3f(-1, 0, 0);
				glTexCoord2f(0.5 * i, 0.2 * j);
				glVertex3f(10, 0.25 * j, i);
				glTexCoord2f(0.5 * i, 0.2 * (j + nr));
				glVertex3f(10, 0.25 * (j + nr), i);
				glTexCoord2f(0.5 * (i + nr), 0.2 * (j + nr));
				glVertex3f(10, 0.25 * (j + nr), i + nr);
				glTexCoord2f(0.5 * (i + nr), 0.2 * j);
				glVertex3f(10, 0.25 * j, i + nr);
			}
		}

		for (i = 0; i < 9.99; i += nr) {
			for (j = 0; j < 9.99; j += nr) {
				// Ceiling
				//glColor3f(i / 10.0, 0, j / 10.0);
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
	glEnable(GL_COLOR_MATERIAL);
}

void drawCube(GLdouble size) 
{
	int i, j;
	GLdouble s = size / 10;
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

void jumpFunc(void) {
	// The player jumps only if the spacebar is pressed and if he is not jumping
	if (!isJumping && keyStates[32]) {
		isJumping = true;

		// Makes a push vector based on input and direction (this allows sprint jumping)
		vector direction = substractv(createv(target.x, EYE_HEIGHT, target.z), eye);
		normalizev(&direction);
		vector push = createv(0, 0, 0);
		if (keyStates['w']) {
			push = addv(push, direction);
		}
		if (keyStates['s']) {
			push = substractv(push, direction);
		}
		if (keyStates['a']) {
			push = addv(push, rotatev(direction, 90, 0, 1, 0));
		}
		if (keyStates['d']) {
			push = addv(push, rotatev(direction, -90, 0, 1, 0));
		}

		// Makes the push vector the right length
		normalizev(&push);
		multiplyv(&push, ACCELERATION * 5);

		// Adds the push vector to the camera force
		cameraForce = addv(cameraForce, push);

		// Makes sure camera force does not exceed MAX_SPEED
		if (vlength(cameraForce) > MAX_SPEED) {
			normalizev(&cameraForce);
			multiplyv(&cameraForce, MAX_SPEED);
		}

		// Adds the jump vector to the camera force
		//                                  GFORCE * (timeOfJump / 2 + 1)
		//                                               ||
		cameraForce = addv(cameraForce, createv(0, GFORCE * 16, 0));
	}
}

void moveCamera(void) 
{
	// Checks whether to jump or not
	jumpFunc();
	
	// Aplys drag to the camera force if he isn't jumping
	if (!isJumping) {
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

	// Aplys air drag to the camera force if he is jumping
	else {
		vector drag = createv(cameraForce.x, 0, cameraForce.z);
		normalizev(&drag);
		if (vlength(createv(cameraForce.x, 0, cameraForce.z)) - AIR_DRAG > 0) {
			multiplyv(&drag, vlength(createv(cameraForce.x, 0, cameraForce.z)) - AIR_DRAG);
		}
		else {
			drag = createv(0, 0, 0);
		}
		cameraForce = createv(drag.x, cameraForce.y, drag.z);
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

	// Makes the sprint vector
	vector sprint = acc;
	normalizev(&sprint);
	multiplyv(&sprint, 2 * ACCELERATION);

	// Adds the acceleration or the sprint vector to the camera force if he isn't jumping and makes sure it does not exeed max speed or max speed + sprint boost
	if (!isJumping) {
		if (!isSprinting && vlength(addv(cameraForce, acc)) <= MAX_SPEED) 
			cameraForce = addv(cameraForce, acc);
		if (isSprinting && vlength(addv(cameraForce, sprint)) <= MAX_SPEED + SPRINT_BOOST)
			cameraForce = addv(cameraForce, sprint);
	}

	// Gravity
	cameraForce = addv(cameraForce, gForce);

	// Check for floor
	if (eye.y + cameraForce.y < EYE_HEIGHT) {
		target.y += EYE_HEIGHT - eye.y;
		eye.y = EYE_HEIGHT;
		cameraForce = createv(cameraForce.x, 0, cameraForce.z);
		isJumping = false;
	}

	// Moves eye and target
	eye = addv(eye, cameraForce);
	target = addv(target, cameraForce);
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
		//glColor3f(0, 1, 1);
		glutSolidTeapot(0.3);
	glPopMatrix();

	drawWallsAndFloor();

	// Draws and animates a green cube
	glPushMatrix();
		GLdouble k = (a2 - ((int) a2 / 90) * 90) * 2 * DEG_TO_RAD;
		glTranslatef(5 + 4 * sin(r), 0.5 + sin(k) * (sqrt(2) / 2 - 0.5), 5 + 4 * cos(r));
		glRotatef(a2, -sin(r), 0, -cos(r));
		glRotatef(a1, 0, 1, 0);
		//glColor3f(0,0.5,0);
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
	// Set the background to light gray
	glClearColor(0.8, 0.8, 0.8, 1);
	glEnable(GL_DEPTH_TEST);
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
	gForce = createv(0, - GFORCE, 0);
	// Loads texture
	textureData = LoadDIBitmap("wall.bmp", &textureInfo);
	textureData2 = LoadDIBitmap("wall2.bmp", &textureInfo2);
}

void freeCameraHandler (int x, int y) {
	// Determines the angle on each axis based on mouse position
	cameraXAngle += -45 + 90 * x / (GLdouble) viewWidth;
	cameraYAngle += -30 + 60 * y / (GLdouble) viewHeight;

	// cameraXAngle must not exeed 360 or be below -360 degrees
	cameraXAngle -= ((int) cameraYAngle / 360) * 360;

	// cameraYAngle must not exeed 90 degrees or be below -90 degrees
	if (cameraYAngle > 89) 
		cameraYAngle = 89;
	if (cameraYAngle < -89) 
		cameraYAngle = -89;

	// Some notations
	GLdouble sinY = sin(-cameraYAngle * DEG_TO_RAD);
	GLdouble sinX = sin(cameraXAngle * DEG_TO_RAD);
	GLdouble cosX = cos(cameraXAngle * DEG_TO_RAD);

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
		a1 -= 360;
	r = a1 * DEG_TO_RAD;

	a2 += 5 * 0.62831;
	if (a2 >= 360)
		a2 -= 360;

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
			keyStates[32] = true;
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
		// Shift key
		case 112:
			isSprinting = true;
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
		case 32:
			keyStates[32] = false;
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
		// Shift key
		case 112:
			isSprinting = false;
			break;
	}
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Epic Game");

	// Event listeners
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(normalKeysHandler);
	glutKeyboardUpFunc(normalKeysUpHandler);
	glutSpecialFunc(specialKeysHandler);
	glutSpecialUpFunc(specialKeysUpHandler);
	glutPassiveMotionFunc(freeCameraHandler);
	glutMotionFunc(freeCameraHandler);
	
	initialize();

	// Hides the mouse cursor
	glutSetCursor(GLUT_CURSOR_NONE);

	// Starts main timer
	glutTimerFunc(10, tick, 0);

	glutMainLoop();
	return 0;
}
