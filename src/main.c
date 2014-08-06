#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../include/utility.h"
#include "../include/bitmap.h"
#include "../include/lobjder.h"

// C does not support boolean?! WTF!
#define true 1
#define false 0
#define EYE_HEIGHT 1.7
#define ACCELERATION 0.02
#define DRAG 0.01
#define AIR_DRAG 0.0025
#define MAX_SPEED 0.10
#define SPRINT_BOOST 0.07
// The formula is: GFORCE = 2 * heightOfJump / ((timeOfJump / 2) * (timeOfJump / 2 + 1))   where heigthOfJump is in meters and timeOfJump is in milliseconds
#define GFORCE 0.00583333

// Ligth parameters
GLfloat lightPos[] = {5, 2.3, 5, 1};
GLfloat lightAmbient[] = {0.2, 0.2, 0.2, 1};
GLfloat lightDiffuse[] = {0.8, 0.8, 0.8, 1};
GLfloat lightSpecular[] = {1.0, 1.0, 1.0, 1};
// Material parameters
GLfloat matAmbient[] = {0.0, 0.0, 0.0, 1}; // << This two don't seem to do anything
GLfloat matDiffuse[] = {0.0, 0.0, 0.0, 1}; // <<
GLfloat matSpecular[] = {0.3, 0.3, 0.3, 1};
GLfloat matEmission[] = {0, 0, 0, 1};
GLfloat matShininess[] = {50.0};

GLdouble a1 = 0, a2 = 0, r;
// Viewport width and height
int viewHeight, viewWidth;
// Camerea pitch and yaw
GLdouble cameraYaw = 225, cameraPitch = 0;
// Vectors
vector eye, target, cameraForce, gForce;
// Key state: true if the key is pressed and false if not
int keyStates[256];
int isJumping = false;
int isSprinting = false;
// Textures
Texture floorTex, wallTex, grassTex;
Model planeModel;


void drawWallsAndFloor(void)
{
	GLdouble i, j;
	// The walls and floor will be a (10 / nr) * (10 / nr) grid
	GLdouble nr = 0.1;

	// Enables textures
	glEnable(GL_TEXTURE_2D);
	// Texture environment
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Binds the floor's texture to GL_TEXTURE_2D
	glBindTexture(GL_TEXTURE_2D, floorTex.texName);
	// Resets the color to white
	glColor3f(1.0,1.0,1.0);
	// Draws floor
	glBegin(GL_QUADS);
		for (i = 0; i < 9.99; i += nr) {
			for (j = 0; j < 9.99; j += nr) {
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

	// Binds the wall's texture to GL_TEXTURE_2D
	glBindTexture(GL_TEXTURE_2D, wallTex.texName);
	// Draws walls
	glBegin(GL_QUADS);
		for (i = 0; i < 9.99; i += nr) {
			for (j = 0; j < 9.99; j += nr) {
				// Wall 1
				glNormal3f(0, 0, 1);
				glTexCoord2f(0.5 * i, 0.125 * j);
				glVertex3f(i, 0.25 * j, 0);
				glTexCoord2f(0.5 * i, 0.125 * (j + nr));
				glVertex3f(i, 0.25 * (j + nr), 0);
				glTexCoord2f(0.5 * (i + nr), 0.125 * (j + nr));
				glVertex3f(i + nr, 0.25 * (j + nr), 0);
				glTexCoord2f(0.5 * (i + nr), 0.125 * j);
				glVertex3f(i + nr, 0.25 * j, 0);

				// Wall 2
				glNormal3f(0, 0, -1);
				glTexCoord2f(0.5 * i, 0.125 * j);
				glVertex3f(i, 0.25 * j, 10);
				glTexCoord2f(0.5 * i, 0.125 * (j + nr));
				glVertex3f(i, 0.25 * (j + nr), 10);
				glTexCoord2f(0.5 * (i + nr), 0.125 * (j + nr));
				glVertex3f(i + nr, 0.25 * (j + nr), 10);
				glTexCoord2f(0.5 * (i + nr), 0.125 * j);
				glVertex3f(i + nr, 0.25 * j, 10);

				// Wall 3
				glNormal3f(1, 0, 0);
				glTexCoord2f(0.5 * i, 0.125 * j);
				glVertex3f(0, 0.25 * j, i);
				glTexCoord2f(0.5 * i, 0.125 * (j + nr));
				glVertex3f(0, 0.25 * (j + nr), i);
				glTexCoord2f(0.5 * (i + nr), 0.125 * (j + nr));
				glVertex3f(0, 0.25 * (j + nr), i + nr);
				glTexCoord2f(0.5 * (i + nr), 0.125 * j);
				glVertex3f(0, 0.25 * j, i + nr);

				// Wall 4
				glNormal3f(-1, 0, 0);
				glTexCoord2f(0.5 * i, 0.125 * j);
				glVertex3f(10, 0.25 * j, i);
				glTexCoord2f(0.5 * i, 0.125 * (j + nr));
				glVertex3f(10, 0.25 * (j + nr), i);
				glTexCoord2f(0.5 * (i + nr), 0.125 * (j + nr));
				glVertex3f(10, 0.25 * (j + nr), i + nr);
				glTexCoord2f(0.5 * (i + nr), 0.125 * j);
				glVertex3f(10, 0.25 * j, i + nr);
			}
		}
	glEnd();

	// Binds the ceiling's texture to GL_TEXTURE_2D
	glBindTexture(GL_TEXTURE_2D, wallTex.texName);
	// Draws ceiling
	glBegin(GL_QUADS);
		for (i = 0; i < 9.99; i += nr) {
			for (j = 0; j < 9.99; j += nr) {
				glNormal3f(0, -1, 0);
				glTexCoord2f(i, j);
				glVertex3f(i, 2.5, j);
				glTexCoord2f(i, j + nr);
				glVertex3f(i, 2.5, j + nr);
				glTexCoord2f(i + nr, j + nr);
				glVertex3f(i + nr, 2.5, j + nr);
				glTexCoord2f(i + nr, j);
				glVertex3f(i + nr, 2.5, j);
			}
		}
	glEnd();
	// Disables textures
	glDisable(GL_TEXTURE_2D);
}

// This will draw a 100 by 100 plane, the camera being always in the middle of it. Gives the impression of an infinite world
void drawGround() {
	int centerX = (int) eye.x, centerY = (int) eye.z;
	int i, j;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, grassTex.texName);
	glPushMatrix();
		glTranslatef(0, -0.01, 0);
		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);
			for (i = 0; i < 100; i++) {
				for (j = 0; j < 100; j++) {
					glTexCoord2f(1, 0);
					glVertex3f(centerX + i - 50, 0, centerY + j - 50);
					glTexCoord2f(1, 1);
					glVertex3f(centerX + i - 50 + 1, 0, centerY + j - 50);
					glTexCoord2f(0, 1);
					glVertex3f(centerX + i - 50 + 1, 0, centerY + j - 50 + 1);
					glTexCoord2f(0, 0);
					glVertex3f(centerX + i - 50, 0, centerY + j - 50 + 1);
				}
			}
		glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
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
		multiplyv(&push, ACCELERATION * 7);

		// Adds the push vector to the camera force and limits its speed
		if (!isSprinting && vlength(addv(cameraForce, push)) <= MAX_SPEED)
			cameraForce = addv(cameraForce, push);
		else if (!isSprinting && vlength(addv(cameraForce, push)) > MAX_SPEED && vlength(cameraForce) <= MAX_SPEED) {
			cameraForce = addv(cameraForce, push);
			normalizev(&cameraForce);
			multiplyv(&cameraForce, MAX_SPEED);
		}
		if (isSprinting && vlength(addv(cameraForce, push)) <= MAX_SPEED + SPRINT_BOOST)
			cameraForce = addv(cameraForce, push);
		else if (isSprinting && vlength(addv(cameraForce, push)) > MAX_SPEED + SPRINT_BOOST && vlength(cameraForce) <= MAX_SPEED + SPRINT_BOOST) {
			cameraForce = addv(cameraForce, push);
			normalizev(&cameraForce);
			multiplyv(&cameraForce, MAX_SPEED + SPRINT_BOOST);
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

	// Adds the acceleration or the sprint vector to the camera force if he isn't jumping and limits its speed
	if (!isJumping) {
		if (!isSprinting && vlength(addv(cameraForce, acc)) <= MAX_SPEED)
			cameraForce = addv(cameraForce, acc);
		else if (!isSprinting && vlength(addv(cameraForce, acc)) > MAX_SPEED && vlength(cameraForce) <= MAX_SPEED) {
			cameraForce = addv(cameraForce, acc);
			normalizev(&cameraForce);
			multiplyv(&cameraForce, MAX_SPEED);
		}
		else if (!isSprinting && vlength(addv(cameraForce, acc)) > MAX_SPEED && vlength(cameraForce) > MAX_SPEED) {
			GLdouble length = vlength(cameraForce);
			cameraForce = addv(cameraForce, acc);
			normalizev(&cameraForce);
			multiplyv(&cameraForce, length);
		}
		if (isSprinting && vlength(addv(cameraForce, sprint)) <= MAX_SPEED + SPRINT_BOOST) 
			cameraForce = addv(cameraForce, sprint);
		else if (isSprinting && vlength(addv(cameraForce, sprint)) > MAX_SPEED + SPRINT_BOOST && vlength(cameraForce) <= MAX_SPEED + SPRINT_BOOST) {
			cameraForce = addv(cameraForce, sprint);
			normalizev(&cameraForce);
			multiplyv(&cameraForce, MAX_SPEED + SPRINT_BOOST);
		}
		else if (isSprinting && vlength(addv(cameraForce, sprint)) > MAX_SPEED + SPRINT_BOOST && vlength(cameraForce) > MAX_SPEED + SPRINT_BOOST) {
			GLdouble length = vlength(cameraForce);
			cameraForce = addv(cameraForce, sprint);
			normalizev(&cameraForce);
			multiplyv(&cameraForce, length);
		}
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

	// Enables color material
	glEnable(GL_COLOR_MATERIAL);

	// Draws and rotates a cyan teapot
	glPushMatrix();
		glTranslatef(2, 0.2, 2);
		glRotatef(a2, 0, 1, 0);
		glColor3f(0, 1, 1);
		glutSolidTeapot(0.3);
	glPopMatrix();

	// Draws and animates a green cube
	glPushMatrix();
		GLdouble k = (a2 - ((int) a2 / 90) * 90) * 2 * DEG_TO_RAD;
		glTranslatef(5 + 4 * sin(r), 0.5 + sin(k) * (sqrt(2) / 2 - 0.5), 5 + 4 * cos(r));
		glRotatef(a2, -sin(r), 0, -cos(r));
		glRotatef(a1, 0, 1, 0);
		glColor3f(0,0.5,0);
		drawCube(1);
	glPopMatrix();

	glColor3f(1, 1, 1);
	// Disables color material
	glDisable(GL_COLOR_MATERIAL);

	drawWallsAndFloor();

	drawGround();

	glPushMatrix();
		glTranslatef(7, 0, 7);
		glRotatef(a1, 0 , 1, 0);
		glRotatef(-90, 1, 0, 0);
		glScalef(0.03, 0.03, 0.03);
		drawModel(&planeModel);
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
	// Enables depth test
	glEnable(GL_DEPTH_TEST);
	// Sets the light
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	// Enables lighting
	glEnable(GL_LIGHTING);
	// Enables the light
	glEnable(GL_LIGHT0);
	// Sets the material ligthing
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, matEmission);
	glShadeModel(GL_SMOOTH);

	// Loads textures

	// Loads the .bmp file
	loadBMP("./data/textures/floor.bmp", &floorTex);
	// Generates a texture name
	glGenTextures(1, &floorTex.texName);
	// Binds the texture to GL_TEXTURE_2D. All the parameters that we set now will be the same when we bind the texture later
	glBindTexture(GL_TEXTURE_2D, floorTex.texName);
	// Sets some parameters
	// Repeat the image on both axes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Sets the interpolation to linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Sets the image to be used as a texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
				floorTex.texWidth, 
				floorTex.texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 
				floorTex.texData);

	// Same as above
	loadBMP("./data/textures/wall.bmp", &wallTex);
	glGenTextures(1, &wallTex.texName);
	glBindTexture(GL_TEXTURE_2D, wallTex.texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
				wallTex.texWidth, 
				wallTex.texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 
				wallTex.texData);

	loadBMP("./data/textures/grass3.bmp", &grassTex); // <<< Not working with grass2.bmp
	glGenTextures(1, &grassTex.texName);
	glBindTexture(GL_TEXTURE_2D, grassTex.texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // <<< Change to GL_LINEAR if not using a Minecraft texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // <<<
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
				grassTex.texWidth, 
				grassTex.texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 
				grassTex.texData);

	//loadOBJToModel("./data/models/plane.obj", &planeModel);
	loadOBJToModel("./data/models/alfa147.obj", &planeModel);

	// Represents the camera position
	eye = createv(5, EYE_HEIGHT, 5);

	// Represents the point where the camera looks
	target = createv(0, EYE_HEIGHT, 0);

	// A vector that moves the camera
	cameraForce = createv(0, 0, 0);

	// Gravity
	gForce = createv(0, - GFORCE, 0);
}

void freeCameraHandler (int x, int y) {
	// Determines the angle on each axis based on mouse position
	cameraYaw += -45 + 90 * x / (GLdouble) viewWidth;
	cameraPitch += -30 + 60 * y / (GLdouble) viewHeight;

	// cameraYaw must not exeed 360 or be below -360 degrees
	cameraYaw -= ((int) cameraYaw / 360) * 360;

	// cameraPitch must not exeed 90 degrees or be below -90 degrees
	if (cameraPitch > 89) 
		cameraPitch = 89;
	if (cameraPitch < -89) 
		cameraPitch = -89;

	// Some notations
	GLdouble sinY = sin(-cameraPitch * DEG_TO_RAD);
	GLdouble sinX = sin(cameraYaw * DEG_TO_RAD);
	GLdouble cosX = cos(cameraYaw * DEG_TO_RAD);

	// Makes the target vector based on cameraYaw and cameraPitch (rotating the vector using rotatev() was a bit buggy)
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
		// Space Key
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
