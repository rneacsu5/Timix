/* 
	This handles camera movement
*/

#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../include/utility.h"
#include "../include/motion.h"

// Camerea pitch and yaw
GLdouble cameraYaw = 225, cameraPitch = 0;
// Vectors
vector eye, target, cameraForce;
// Key state: true if the key is pressed and false if not
int keyStates[256];
int isJumping = false;
int isSprinting = false;

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

void motMoveCamera(void) 
{
	GLint v[4];
	glGetIntegerv(GL_VIEWPORT, v);
	// Moves mouse to the middle
	glutWarpPointer(v[2] / 2, v[3] / 2);

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
	cameraForce = addv(cameraForce, createv(0, - GFORCE, 0));

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

	gluLookAt(eye.x, eye.y, eye.z, target.x, target.y, target.z, 0, 1, 0);
}

void freeCameraHandler (int x, int y) {
	GLint v[4];
	glGetIntegerv(GL_VIEWPORT, v); // Gets 4 values: X, Y position of window, Width and Heigth of viewport
	// Determines the angle on each axis based on mouse position
	cameraYaw += -45 + 90 * x / (GLdouble) v[2];
	cameraPitch += -30 + 60 * y / (GLdouble) v[3];

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

void motionInit(void) {
	glutKeyboardFunc(normalKeysHandler);
	glutKeyboardUpFunc(normalKeysUpHandler);
	glutSpecialFunc(specialKeysHandler);
	glutSpecialUpFunc(specialKeysUpHandler);
	glutPassiveMotionFunc(freeCameraHandler);
	glutMotionFunc(freeCameraHandler);

	// Represents the camera position
	eye = createv(5, EYE_HEIGHT, 5);

	// Represents the point where the camera looks
	target = createv(0, EYE_HEIGHT, 0);

	// A vector that moves the camera
	cameraForce = createv(0, 0, 0);

	// Hides the mouse cursor
	glutSetCursor(GLUT_CURSOR_NONE);
}

vector motGetEyePos(void) {
	return eye;
}

vector motGetTargetPos(void) {
	return target;
}

vector motGetCamForce(void) {
	return cameraForce;
}

int motGetKeyStatus(int key) {
	return keyStates[key];
}

int motGetJump(void) {
	return isJumping;
}

int motGetSprint(void) {
	return isSprinting;
}