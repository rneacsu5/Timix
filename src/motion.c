/* 
	This handles camera movement
*/

#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../include/motion.h"

motVector motCreatev(GLdouble x, GLdouble y, GLdouble z) 
{
	motVector v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

void motNormalizev(motVector *v) {

	GLdouble modul = sqrt((*v).x * (*v).x + (*v).y * (*v).y + (*v).z * (*v).z);
	if (modul != 0) {
		(*v).x /= modul;
		(*v).y /= modul;
		(*v).z /= modul; 
	}
}

void motMultiplyv(motVector *v, GLdouble factor) 
{
	(*v).x *= factor;
	(*v).y *= factor;
	(*v).z *= factor;
}

motVector motAddv(motVector v1, motVector v2) 
{
	motVector sum;
	sum.x = v1.x + v2.x;
	sum.y = v1.y + v2.y;
	sum.z = v1.z + v2.z;
	return sum;
}

motVector motSubstractv(motVector v1, motVector v2) 
{
	motVector diff;
	diff.x = v1.x - v2.x;
	diff.y = v1.y - v2.y;
	diff.z = v1.z - v2.z;
	return diff;
}

motVector motRotatev(motVector v, GLdouble angle, GLdouble x, GLdouble y, GLdouble z) 
{
	// Normalizes the rotation motVector
	motVector n = motCreatev(x, y, z);
	motNormalizev(&n);
	x = n.x;
	y = n.y;
	z = n.z;

	GLdouble r = angle * DEG_TO_RAD;
	GLdouble c = cos(r);
	GLdouble s = sin(r);

	// Rotates the motVector
	motVector out;
	out.x = (x * x * (1 - c) + c) * v.x + (x * y * (1 - c) - z * s) * v.y + (x * z * (1 - c) + y * s) * v.z;
	out.y = (y * x * (1 - c) + z * s) * v.x + (y * y * (1 - c) + c) * v.y + (y * z * (1 - c) - x * s) * v.z;
	out.z = (x * z * (1 - c) - y * s) * v.x + (y * z * (1 - c) + x * s) * v.y + (z * z * (1 - c) + c) * v.z;

	return out;
}


GLdouble motvLength(motVector v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Camerea pitch and yaw
GLdouble cameraYaw = 0, cameraPitch = 0;
// Vectors
motVector eye, target, cameraForce;
// Key state: true if the key is pressed and false if not
int keyStates[256];
int isJumping = false;
int isSprinting = false;

void jumpFunc(void) {
	// The player jumps only if the spacebar is pressed and if he is not jumping
	if (!isJumping && keyStates[32]) {
		isJumping = true;

		// Makes a push motVector based on input and direction (this allows sprint jumping)
		motVector direction = motSubstractv(motCreatev(target.x, EYE_HEIGHT, target.z), eye);
		motNormalizev(&direction);
		motVector push = motCreatev(0, 0, 0);
		if (keyStates['w']) {
			push = motAddv(push, direction);
		}
		if (keyStates['s']) {
			push = motSubstractv(push, direction);
		}
		if (keyStates['a']) {
			push = motAddv(push, motRotatev(direction, 90, 0, 1, 0));
		}
		if (keyStates['d']) {
			push = motAddv(push, motRotatev(direction, -90, 0, 1, 0));
		}

		// Makes the push motVector the right length
		motNormalizev(&push);
		motMultiplyv(&push, ACCELERATION * 7);

		// Adds the push motVector to the camera force and limits its speed
		if (!isSprinting && motvLength(motAddv(cameraForce, push)) <= MAX_SPEED)
			cameraForce = motAddv(cameraForce, push);
		else if (!isSprinting && motvLength(motAddv(cameraForce, push)) > MAX_SPEED && motvLength(cameraForce) <= MAX_SPEED) {
			cameraForce = motAddv(cameraForce, push);
			motNormalizev(&cameraForce);
			motMultiplyv(&cameraForce, MAX_SPEED);
		}
		if (isSprinting && motvLength(motAddv(cameraForce, push)) <= MAX_SPEED + SPRINT_BOOST)
			cameraForce = motAddv(cameraForce, push);
		else if (isSprinting && motvLength(motAddv(cameraForce, push)) > MAX_SPEED + SPRINT_BOOST && motvLength(cameraForce) <= MAX_SPEED + SPRINT_BOOST) {
			cameraForce = motAddv(cameraForce, push);
			motNormalizev(&cameraForce);
			motMultiplyv(&cameraForce, MAX_SPEED + SPRINT_BOOST);
		}

		// Adds the jump motVector to the camera force
		//                                  GFORCE * (timeOfJump / 2 + 1)
		//                                               ||
		cameraForce = motAddv(cameraForce, motCreatev(0, GFORCE * 16, 0));
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
		motVector drag = cameraForce;
		motNormalizev(&drag);
		if (motvLength(cameraForce) - DRAG > 0) {
			motMultiplyv(&drag, motvLength(cameraForce) - DRAG);
		}
		else {
			drag = motCreatev(0, 0, 0);
		}
		cameraForce = drag;
	}

	// Aplys air drag to the camera force if he is jumping
	else {
		motVector drag = motCreatev(cameraForce.x, 0, cameraForce.z);
		motNormalizev(&drag);
		if (motvLength(motCreatev(cameraForce.x, 0, cameraForce.z)) - AIR_DRAG > 0) {
			motMultiplyv(&drag, motvLength(motCreatev(cameraForce.x, 0, cameraForce.z)) - AIR_DRAG);
		}
		else {
			drag = motCreatev(0, 0, 0);
		}
		cameraForce = motCreatev(drag.x, cameraForce.y, drag.z);
	}

	// Makes the acceleration motVector based on input and direction
	motVector direction = motSubstractv(motCreatev(target.x, EYE_HEIGHT, target.z), eye);
	motNormalizev(&direction);
	motVector acc = motCreatev(0, 0, 0);
	if (keyStates['w']) {
		acc = motAddv(acc, direction);
	}
	if (keyStates['s']) {
		acc = motSubstractv(acc, direction);
	}
	if (keyStates['a']) {
		acc = motAddv(acc, motRotatev(direction, 90, 0, 1, 0));
	}
	if (keyStates['d']) {
		acc = motAddv(acc, motRotatev(direction, -90, 0, 1, 0));
	}

	// Makes the acceleration motVector the right length
	motNormalizev(&acc);
	motMultiplyv(&acc, ACCELERATION);

	// Makes the sprint motVector
	motVector sprint = acc;
	motNormalizev(&sprint);
	motMultiplyv(&sprint, 2 * ACCELERATION);

	// Adds the acceleration or the sprint motVector to the camera force if he isn't jumping and limits its speed
	if (!isJumping) {
		if (!isSprinting && motvLength(motAddv(cameraForce, acc)) <= MAX_SPEED)
			cameraForce = motAddv(cameraForce, acc);
		else if (!isSprinting && motvLength(motAddv(cameraForce, acc)) > MAX_SPEED && motvLength(cameraForce) <= MAX_SPEED) {
			cameraForce = motAddv(cameraForce, acc);
			motNormalizev(&cameraForce);
			motMultiplyv(&cameraForce, MAX_SPEED);
		}
		else if (!isSprinting && motvLength(motAddv(cameraForce, acc)) > MAX_SPEED && motvLength(cameraForce) > MAX_SPEED) {
			GLdouble length = motvLength(cameraForce);
			cameraForce = motAddv(cameraForce, acc);
			motNormalizev(&cameraForce);
			motMultiplyv(&cameraForce, length);
		}
		if (isSprinting && motvLength(motAddv(cameraForce, sprint)) <= MAX_SPEED + SPRINT_BOOST) 
			cameraForce = motAddv(cameraForce, sprint);
		else if (isSprinting && motvLength(motAddv(cameraForce, sprint)) > MAX_SPEED + SPRINT_BOOST && motvLength(cameraForce) <= MAX_SPEED + SPRINT_BOOST) {
			cameraForce = motAddv(cameraForce, sprint);
			motNormalizev(&cameraForce);
			motMultiplyv(&cameraForce, MAX_SPEED + SPRINT_BOOST);
		}
		else if (isSprinting && motvLength(motAddv(cameraForce, sprint)) > MAX_SPEED + SPRINT_BOOST && motvLength(cameraForce) > MAX_SPEED + SPRINT_BOOST) {
			GLdouble length = motvLength(cameraForce);
			cameraForce = motAddv(cameraForce, sprint);
			motNormalizev(&cameraForce);
			motMultiplyv(&cameraForce, length);
		}
	}

	// Gravity
	cameraForce = motAddv(cameraForce, motCreatev(0, - GFORCE, 0));

	// Check for floor
	if (eye.y + cameraForce.y < EYE_HEIGHT) {
		target.y += EYE_HEIGHT - eye.y;
		eye.y = EYE_HEIGHT;
		cameraForce = motCreatev(cameraForce.x, 0, cameraForce.z);
		isJumping = false;
	}

	// Moves eye and target
	eye = motAddv(eye, cameraForce);
	target = motAddv(target, cameraForce);

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

	// Makes the target motVector based on cameraYaw and cameraPitch (rotating the motVector using motRotatev() was a bit buggy)
	target = motCreatev(cosX, 0, sinX);
	motMultiplyv(&target, sqrt(1 - sinY * sinY));
	target = motAddv(target, motCreatev(0, sinY, 0));

	// Adds the eye position so that the camera points to the right place
	target = motAddv(target, eye);	
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
	eye = motCreatev(5, EYE_HEIGHT, 5);

	// Represents the point where the camera looks
	target = motCreatev(0, EYE_HEIGHT, 0);

	// A motVector that moves the camera
	cameraForce = motCreatev(0, 0, 0);

	// Hides the mouse cursor
	glutSetCursor(GLUT_CURSOR_NONE);
}

motVector motGetEyePos(void) {
	return eye;
}

motVector motGetTargetPos(void) {
	return target;
}

motVector motGetCamForce(void) {
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