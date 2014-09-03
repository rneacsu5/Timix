/*
	motion library 
	Realistic camera movement for OpenGL
	Copyright (c) 2014 Neacsu Razvan
*/

#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h> 
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

motVector motMultiplyv(motVector v, GLdouble factor) 
{
	motVector out = v;
	out.x *= factor;
	out.y *= factor;
	out.z *= factor;
	return out;
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

	GLdouble r = angle * MOT_DEG_TO_RAD;
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
motVector eye, target, speed;
// Key state: true if the key is pressed and false if not
int keyStates[256];
int isJumping = false;
int isSprinting = false;
int isPaused = false;

// The time interval between callbacks in seconds
GLfloat deltaT;

// Not used
clock_t currentTime;
int firstTime = 1;

void jumpFunc(void) {
	// The player jumps only if the spacebar is pressed and if he is not jumping
	if (!isJumping && keyStates[32]) {
		isJumping = true;

		// Makes a push motVector based on input and direction (this allows sprint jumping)
		motVector direction = motSubstractv(motCreatev(target.x, eye.y, target.z), eye);
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
		push = motMultiplyv(push, 5 * MOT_ACCELERATION * deltaT);

		// Adds the push motVector to the camera speed and limits its speed
		if (!isSprinting && motvLength(motAddv(motCreatev(speed.x, 0, speed.z), push)) <= MOT_JUMP_MAX_SPEED) {
			speed = motAddv(speed, push);
		}
		else if (!isSprinting && motvLength(motAddv(motCreatev(speed.x, 0, speed.z), push)) > MOT_JUMP_MAX_SPEED && motvLength(motCreatev(speed.x, 0, speed.z)) <= MOT_JUMP_MAX_SPEED) {
			speed = motAddv(speed, push);
			motNormalizev(&speed);
			speed = motMultiplyv(speed, MOT_JUMP_MAX_SPEED);
		}
		if (isSprinting && motvLength(motAddv(motCreatev(speed.x, 0, speed.z), push)) <= MOT_JUMP_SPRINT_MAX_SPEED)
			speed = motAddv(speed, push);
		else if (isSprinting && motvLength(motAddv(motCreatev(speed.x, 0, speed.z), push)) > MOT_JUMP_SPRINT_MAX_SPEED && motvLength(motCreatev(speed.x, 0, speed.z)) <= MOT_JUMP_SPRINT_MAX_SPEED) {
			speed = motAddv(speed, push);
			motNormalizev(&speed);
			speed = motMultiplyv(speed, MOT_JUMP_SPRINT_MAX_SPEED);
		}

		speed = motAddv(speed, motCreatev(0, MOT_JUMP_SPEED, 0));
	}
}

void motMoveCamera(void) {
	// If paused
	if (isPaused) {
		// Set up camera
		gluLookAt(eye.x, eye.y, eye.z, target.x, target.y, target.z, 0, 1, 0);
		return;
	}

	// Moves mouse to the middle
	GLint v[4];
	glGetIntegerv(GL_VIEWPORT, v);
	glutWarpPointer(v[2] / 2, v[3] / 2);
 
	// clock_t oldTime = currentTime;
	// currentTime = clock();
	// if (firstTime) {
	// 	firstTime = 0;
	// 	deltaT = 0;
	// }
	// else
	// 	deltaT = ((GLfloat) currentTime - oldTime) / CLOCKS_PER_SEC;
	// printf("%f\n", deltaT);

	// The following two kinematic equations are used: d = v0 * deltaT + (a * deltaT ^ 2) / 2 and v = v0 + a * deltaT
	// where v0 is the initial speed, v is the final speed, a is the acceleration, deltaT is the time interval and d is the distance traveled
	// Note that the previous are applied for 3D vectors (v0, v, d and a are 3D vectors)

	// Time interval is hardcoded for my computer
	deltaT = 1 / 60.0;

	// Initial speed
	motVector speed0 = speed;

	// Brake acceleration  (opposite to the speed and parallel to the ground)
	motVector drag = motCreatev(speed0.x, 0, speed0.z);
	motNormalizev(&drag);
	drag = motMultiplyv(drag, -MOT_BRAKE_ACCELERATION);
	if (!isJumping) {
		if (motvLength(speed) > motvLength(motMultiplyv(drag, deltaT))) {
			speed = motAddv(speed, motMultiplyv(drag, deltaT));
		}
		else {
			speed = motCreatev(0, 0, 0);
		}
	}

	// Checks whether to jump or not
	jumpFunc();

	// Air drag acceleration (opposite to the speed)
	motVector airDrag = speed0;
	motNormalizev(&airDrag);
	// Air drag is proportional to the speed
	airDrag = motMultiplyv(airDrag, - motvLength(speed0) * MOT_AIR_DRAG);
	if (motvLength(speed) > motvLength(motMultiplyv(airDrag, deltaT))) {
		speed = motAddv(speed, motMultiplyv(airDrag, deltaT));
	}
	else {
		speed = motCreatev(0, 0, 0);
	}

	// Gravity
	speed = motAddv(speed, motMultiplyv(motCreatev(0, - MOT_GFORCE, 0), deltaT));

	// Makes the acceleration based on input and direction
	motVector direction = motSubstractv(motCreatev(target.x, eye.y, target.z), eye);
	motNormalizev(&direction);
	motVector acc = motCreatev(0, 0, 0);
	if (!isJumping) {
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
	}

	// Makes the acceleration the right length
	motNormalizev(&acc);
	acc = motMultiplyv(acc, MOT_ACCELERATION);

	// If sprinting, resize the acceleration
	if (isSprinting) {
		motNormalizev(&acc);
		acc = motMultiplyv(acc, MOT_SPRINT_ACCELERATION);
	}

	speed = motAddv(speed, motMultiplyv(acc, deltaT));

	int isMoving = keyStates['w'] || keyStates['a'] || keyStates['s'] || keyStates['d'];

	// This part limits the speed of the camera
	if (!isJumping && isMoving) {
		if (!isSprinting && motvLength(motCreatev(speed0.x, 0, speed0.z)) < MOT_MAX_SPEED && motvLength(motCreatev(speed.x, 0, speed.z)) > MOT_MAX_SPEED) {
			motVector s = motCreatev(speed.x, 0, speed.z);
			motNormalizev(&s);
			s = motMultiplyv(s, MOT_MAX_SPEED);
			speed = motCreatev(s.x, speed.y, s.z);
		}
		else if (!isSprinting && motvLength(motCreatev(speed0.x, 0, speed0.z)) >= MOT_MAX_SPEED) {
			motVector speed1 = speed0;
			speed1 = motAddv(speed1, motMultiplyv(drag, deltaT));
			speed1 = motAddv(speed1, motMultiplyv(airDrag, deltaT));
			GLdouble length = motvLength(speed1);
			motNormalizev(&speed);
			speed = motMultiplyv(speed, length);
			if (motvLength(motCreatev(speed.x, 0, speed.z)) < MOT_MAX_SPEED) {
				motVector s = motCreatev(speed.x, 0, speed.z);
				motNormalizev(&s);
				s = motMultiplyv(s, MOT_MAX_SPEED);
				speed = motCreatev(s.x, speed.y, s.z);
			}
		}
		if (isSprinting && motvLength(motCreatev(speed0.x, 0, speed0.z)) < MOT_SPRINT_MAX_SPEED && motvLength(motCreatev(speed.x, 0, speed.z)) > MOT_SPRINT_MAX_SPEED) {
			motVector s = motCreatev(speed.x, 0, speed.z);
			motNormalizev(&s);
			s = motMultiplyv(s, MOT_SPRINT_MAX_SPEED);
			speed = motCreatev(s.x, speed.y, s.z);
		}
		else if (isSprinting && motvLength(motCreatev(speed0.x, 0, speed0.z)) >= MOT_SPRINT_MAX_SPEED) {
			motVector speed1 = speed0;
			speed1 = motAddv(speed1, motMultiplyv(drag, deltaT));
			speed1 = motAddv(speed1, motMultiplyv(airDrag, deltaT));
			GLdouble length = motvLength(speed1);
			motNormalizev(&speed);
			speed = motMultiplyv(speed, length);
			if (motvLength(motCreatev(speed.x, 0, speed.z)) < MOT_SPRINT_MAX_SPEED) {
				motVector s = motCreatev(speed.x, 0, speed.z);
				motNormalizev(&s);
				s = motMultiplyv(s, MOT_SPRINT_MAX_SPEED);
				speed = motCreatev(s.x, speed.y, s.z);
			}
		}
	}

	// The distance traveled
	motVector d = motAddv(motMultiplyv(speed0, deltaT), motMultiplyv(motSubstractv(speed, speed0), deltaT / 2.0));

	// Moves eye and target
	eye = motAddv(eye, d);
	target = motAddv(target, d);

	// Check for floor
	if (eye.y < MOT_EYE_HEIGHT) {
		target.y += MOT_EYE_HEIGHT - eye.y;
		eye.y = MOT_EYE_HEIGHT;
		isJumping = false;
		speed = motCreatev(speed.x, 0, speed.z);
	}

	gluLookAt(eye.x, eye.y, eye.z, target.x, target.y, target.z, 0, 1, 0);
}

void freeCameraHandler (int x, int y) {
	// If paused, don't do anything
	if (isPaused) {
		return;
	}

	GLint v[4];
	glGetIntegerv(GL_VIEWPORT, v); // Gets 4 values: X, Y position of window, Width and Heigth of viewport
	// Determines the angle on each axis based on mouse position
	cameraYaw += -45 + 90 * x / (GLdouble) v[2];
	cameraPitch += -30 + 60 * y / (GLdouble) v[3];

	// cameraYaw must not exeed 360 or be below -360 degrees
	cameraYaw -= ((int) cameraYaw / 360) * 360;

	// cameraPitch must not exeed 89 degrees or be below -89 degrees
	if (cameraPitch > 89) {
		cameraPitch = 89;
	}
	if (cameraPitch < -89) {
		cameraPitch = -89;
	}

	// Some notations
	GLdouble sinY = sin(-cameraPitch * MOT_DEG_TO_RAD);
	GLdouble sinX = sin(cameraYaw * MOT_DEG_TO_RAD);
	GLdouble cosX = cos(cameraYaw * MOT_DEG_TO_RAD);

	// Makes the target vector based on cameraYaw and cameraPitch (rotating the vector using motRotatev() is a bit buggy)
	target = motCreatev(cosX, 0, sinX);
	target = motMultiplyv(target, sqrt(1 - sinY * sinY));
	target = motAddv(target, motCreatev(0, sinY, 0));

	// Adds the eye position so that the camera points to the right place
	target = motAddv(target, eye);	
}

void motTeleportCamera(GLdouble x, GLdouble y, GLdouble z) {
	target.x -= eye.x - x;
	target.y -= eye.y - y;
	target.z -= eye.z - z;
	eye.x = x;
	eye.y = y;
	eye.z = z;
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
		case 'P':
		case 'p':
			isPaused = !isPaused;
			if (isPaused) {
				// Normal cursor
				glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
			}
			else {
				// Hidden cursor
				glutSetCursor(GLUT_CURSOR_NONE);
			}
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
	eye = motCreatev(0, MOT_EYE_HEIGHT, 0);

	// Represents the point where the camera looks
	target = motCreatev(0, MOT_EYE_HEIGHT, 0);

	// The camera speed
	speed = motCreatev(0, 0, 0);

	// Hides the mouse cursor
	glutSetCursor(GLUT_CURSOR_NONE);
}

motVector motGetEyePos(void) {
	return eye;
}

motVector motGetTargetPos(void) {
	return target;
}

int motGetKeyStatus(int key) {
	return keyStates[key];
}

int motGetIsJumping(void) {
	return isJumping;
}

int motGetIsSprinting(void) {
	return isSprinting;
}

int motGetIsPaused(void) {
	return isPaused;
}