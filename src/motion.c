/*
	motion library 
	Realistic camera movement for OpenGL
	Copyright (c) 2014 Neacsu Razvan
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <GL/glut.h>
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32


#include "../include/motion.h"

// Some basic vector operations
mot_Vector motCreatev(GLdouble x, GLdouble y, GLdouble z) 
{
	mot_Vector v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

void motNormalizev(mot_Vector *v) {

	GLdouble modul = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
	if (modul != 0) {
		(*v).x /= modul;
		(*v).y /= modul;
		(*v).z /= modul; 
	}
}

mot_Vector motMultiplyv(mot_Vector v, GLdouble factor) 
{
	mot_Vector out = v;
	out.x *= factor;
	out.y *= factor;
	out.z *= factor;
	return out;
}

mot_Vector motAddv(mot_Vector v1, mot_Vector v2) 
{
	mot_Vector sum;
	sum.x = v1.x + v2.x;
	sum.y = v1.y + v2.y;
	sum.z = v1.z + v2.z;
	return sum;
}

mot_Vector motSubstractv(mot_Vector v1, mot_Vector v2) 
{
	mot_Vector diff;
	diff.x = v1.x - v2.x;
	diff.y = v1.y - v2.y;
	diff.z = v1.z - v2.z;
	return diff;
}

mot_Vector motRotatev(mot_Vector v, GLdouble angle, GLdouble x, GLdouble y, GLdouble z) 
{
	// Normalizes the rotation motVector
	mot_Vector n = motCreatev(x, y, z);
	motNormalizev(&n);
	x = n.x;
	y = n.y;
	z = n.z;

	GLdouble r = angle * MOT_DEG_TO_RAD;
	GLdouble c = cos(r);
	GLdouble s = sin(r);

	// Rotates the motVector
	mot_Vector out;
	out.x = (x * x * (1 - c) + c) * v.x + (x * y * (1 - c) - z * s) * v.y + (x * z * (1 - c) + y * s) * v.z;
	out.y = (y * x * (1 - c) + z * s) * v.x + (y * y * (1 - c) + c) * v.y + (y * z * (1 - c) - x * s) * v.z;
	out.z = (x * z * (1 - c) - y * s) * v.x + (y * z * (1 - c) + x * s) * v.y + (z * z * (1 - c) + c) * v.z;

	return out;
}

GLdouble motvLength(mot_Vector v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Constants

// Eye height
GLdouble EYE_HEIGHT = 1.7;
// Running acceleration
GLdouble ACCELERATION = 40;
// Sprinting acceleration
GLdouble SPRINT_ACCELERATION = 50;
// Braking acceleration
GLdouble BRAKE_ACCELERATION = 25;
// Air drag acceleration coefficient
GLdouble AIR_DRAG = 0.2;
// Maximum running speed
GLdouble MAX_SPEED = 5;
// Maximum sprinting speed
GLdouble SPRINT_MAX_SPEED = 7.5;
// Maximum forward speed while jumping
GLdouble JUMP_MAX_SPEED = 5 / 1.3;
// Maximum forward speed while jumping and sprinting
GLdouble JUMP_SPRINT_MAX_SPEED = 7.5 / 1.3;
// The jump up speed
GLdouble JUMP_SPEED = 3.13;
// Gravitational acceleration
GLdouble GFORCE = 9.81;

// Camerea pitch and yaw
GLdouble cameraYaw = 0, cameraPitch = 0;
// Vectors
mot_Vector eye, target, speed;
// Key states for moving and pausing the game: true if the key is pressed and false if not
int keyStates[256];
// All the key states, used with mot_GetKeyState()
int globalKeyStates[256];
// Other states
int isJumping = mot_false;
int isSprinting = mot_false;
int isPaused = mot_false;
int isOP = mot_false;

// The time interval between callbacks in seconds
GLfloat deltaT = 1 / 100.0;

// Moves camera freely with no gravity
void moveOPCamera(void)
{
	// Moves mouse to the middle
	GLint v[4];
	glGetIntegerv(GL_VIEWPORT, v);
	glutWarpPointer(v[2] / 2, v[3] / 2);

	// Direction in which the camera looks parallel to the ground
	mot_Vector direction = motSubstractv(motCreatev(target.x, eye.y, target.z), eye);
	motNormalizev(&direction);

	mot_Vector speed = motCreatev(0, 0, 0);

	if (keyStates['w']) {
		speed = motAddv(speed, direction);
	}
	if (keyStates['s']) {
		speed = motSubstractv(speed, direction);
	}
	if (keyStates['a']) {
		speed = motAddv(speed, motRotatev(direction, 90, 0, 1, 0));
	}
	if (keyStates['d']) {
		speed = motAddv(speed, motRotatev(direction, -90, 0, 1, 0));
	}

	// Makes the acceleration the right length
	motNormalizev(&speed);
	speed = motMultiplyv(speed, MAX_SPEED * 1.5);

	// Moves up or down
	if (keyStates[32]) {
		speed = motAddv(speed, motCreatev(0, MAX_SPEED, 0));
	}
	if (isSprinting) {
		speed = motAddv(speed, motCreatev(0, -MAX_SPEED, 0));
	}


	// Moves eye and target
	eye = motAddv(eye, motMultiplyv(speed, deltaT));
	target = motAddv(target, motMultiplyv(speed, deltaT));
	gluLookAt(eye.x, eye.y, eye.z, target.x, target.y, target.z, 0, 1, 0);
}

// Checks for the space bar being pressed amd jumps if needed
void jumpFunc(void) {
	// The player jumps only if the spacebar is pressed and if he is not jumping
	if (!isJumping && keyStates[32]) {
		isJumping = mot_true;

		// Makes a push motVector based on input and direction (this allows sprint jumping)
		mot_Vector direction = motSubstractv(motCreatev(target.x, eye.y, target.z), eye);
		motNormalizev(&direction);
		mot_Vector push = motCreatev(0, 0, 0);
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
		push = motMultiplyv(push, 5 * ACCELERATION * deltaT);

		// Adds the push motVector to the camera speed and limits its speed
		if (!isSprinting && motvLength(motAddv(motCreatev(speed.x, 0, speed.z), push)) <= JUMP_MAX_SPEED) {
			speed = motAddv(speed, push);
		}
		else if (!isSprinting && motvLength(motAddv(motCreatev(speed.x, 0, speed.z), push)) > JUMP_MAX_SPEED && motvLength(motCreatev(speed.x, 0, speed.z)) <= JUMP_MAX_SPEED) {
			speed = motAddv(speed, push);
			motNormalizev(&speed);
			speed = motMultiplyv(speed, JUMP_MAX_SPEED);
		}
		if (isSprinting && motvLength(motAddv(motCreatev(speed.x, 0, speed.z), push)) <= JUMP_SPRINT_MAX_SPEED)
			speed = motAddv(speed, push);
		else if (isSprinting && motvLength(motAddv(motCreatev(speed.x, 0, speed.z), push)) > JUMP_SPRINT_MAX_SPEED && motvLength(motCreatev(speed.x, 0, speed.z)) <= JUMP_SPRINT_MAX_SPEED) {
			speed = motAddv(speed, push);
			motNormalizev(&speed);
			speed = motMultiplyv(speed, JUMP_SPRINT_MAX_SPEED);
		}

		speed = motAddv(speed, motCreatev(0, JUMP_SPEED, 0));
	}
}

// Moves the camera one step
void mot_MoveCamera(void) {
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

#ifdef _WIN32
	// Check for shift key if on Windows
	// On Linux there is a code for the shift key
	if ((GetKeyState(VK_LSHIFT) | GetKeyState(VK_RSHIFT)) < 0)
		isSprinting = mot_true;
	else
		isSprinting = mot_false;
#endif // _WIN32

	if (isOP) {
		moveOPCamera();
		return;
	}

	// The following two kinematic equations are used: d = v0 * deltaT + (a * deltaT ^ 2) / 2 and v = v0 + a * deltaT
	// where v0 is the initial speed, v is the final speed, a is the acceleration, deltaT is the time interval and d is the distance traveled
	// Note that the previous are applied for 3D vectors (v0, v, d and a are 3D vectors)

	// Initial speed
	mot_Vector speed0 = speed;

	// Brake acceleration  (opposite to the speed and parallel to the ground)
	mot_Vector drag = motCreatev(speed0.x, 0, speed0.z);
	motNormalizev(&drag);
	drag = motMultiplyv(drag, -BRAKE_ACCELERATION);
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
	mot_Vector airDrag = speed0;
	motNormalizev(&airDrag);
	// Air drag is proportional to the speed
	airDrag = motMultiplyv(airDrag, - motvLength(speed0) * AIR_DRAG);
	if (motvLength(speed) > motvLength(motMultiplyv(airDrag, deltaT))) {
		speed = motAddv(speed, motMultiplyv(airDrag, deltaT));
	}
	else {
		speed = motCreatev(0, 0, 0);
	}

	// Gravity
	speed = motAddv(speed, motMultiplyv(motCreatev(0, - GFORCE, 0), deltaT));

	// Makes the acceleration based on input and direction
	mot_Vector direction = motSubstractv(motCreatev(target.x, eye.y, target.z), eye);
	motNormalizev(&direction);
	mot_Vector acc = motCreatev(0, 0, 0);
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
	acc = motMultiplyv(acc, ACCELERATION);

	// If sprinting, resize the acceleration
	if (isSprinting) {
		motNormalizev(&acc);
		acc = motMultiplyv(acc, SPRINT_ACCELERATION);
	}

	speed = motAddv(speed, motMultiplyv(acc, deltaT));

	int isMoving = keyStates['w'] || keyStates['a'] || keyStates['s'] || keyStates['d'];

	// This part limits the speed of the camera
	if (!isJumping && isMoving) {
		if (!isSprinting && motvLength(motCreatev(speed0.x, 0, speed0.z)) < MAX_SPEED && motvLength(motCreatev(speed.x, 0, speed.z)) > MAX_SPEED) {
			mot_Vector s = motCreatev(speed.x, 0, speed.z);
			motNormalizev(&s);
			s = motMultiplyv(s, MAX_SPEED);
			speed = motCreatev(s.x, speed.y, s.z);
		}
		else if (!isSprinting && motvLength(motCreatev(speed0.x, 0, speed0.z)) >= MAX_SPEED) {
			mot_Vector speed1 = speed0;
			speed1 = motAddv(speed1, motMultiplyv(drag, deltaT));
			speed1 = motAddv(speed1, motMultiplyv(airDrag, deltaT));
			GLdouble length = motvLength(speed1);
			motNormalizev(&speed);
			speed = motMultiplyv(speed, length);
			if (motvLength(motCreatev(speed.x, 0, speed.z)) < MAX_SPEED) {
				mot_Vector s = motCreatev(speed.x, 0, speed.z);
				motNormalizev(&s);
				s = motMultiplyv(s, MAX_SPEED);
				speed = motCreatev(s.x, speed.y, s.z);
			}
		}
		if (isSprinting && motvLength(motCreatev(speed0.x, 0, speed0.z)) < SPRINT_MAX_SPEED && motvLength(motCreatev(speed.x, 0, speed.z)) > SPRINT_MAX_SPEED) {
			mot_Vector s = motCreatev(speed.x, 0, speed.z);
			motNormalizev(&s);
			s = motMultiplyv(s, SPRINT_MAX_SPEED);
			speed = motCreatev(s.x, speed.y, s.z);
		}
		else if (isSprinting && motvLength(motCreatev(speed0.x, 0, speed0.z)) >= SPRINT_MAX_SPEED) {
			mot_Vector speed1 = speed0;
			speed1 = motAddv(speed1, motMultiplyv(drag, deltaT));
			speed1 = motAddv(speed1, motMultiplyv(airDrag, deltaT));
			GLdouble length = motvLength(speed1);
			motNormalizev(&speed);
			speed = motMultiplyv(speed, length);
			if (motvLength(motCreatev(speed.x, 0, speed.z)) < SPRINT_MAX_SPEED) {
				mot_Vector s = motCreatev(speed.x, 0, speed.z);
				motNormalizev(&s);
				s = motMultiplyv(s, SPRINT_MAX_SPEED);
				speed = motCreatev(s.x, speed.y, s.z);
			}
		}
	}

	// The distance traveled
	mot_Vector d = motAddv(motMultiplyv(speed0, deltaT), motMultiplyv(motSubstractv(speed, speed0), deltaT / 2.0));

	// Moves eye and target
	eye = motAddv(eye, d);
	target = motAddv(target, d);

	// Check for floor
	if (eye.y < EYE_HEIGHT) {
		target.y += EYE_HEIGHT - eye.y;
		eye.y = EYE_HEIGHT;
		isJumping = mot_false;
		speed = motCreatev(speed.x, 0, speed.z);
	}

	gluLookAt(eye.x, eye.y, eye.z, target.x, target.y, target.z, 0, 1, 0);
}

// Handels mouse movements
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

// Teleports the camera
void mot_TeleportCamera(GLdouble x, GLdouble y, GLdouble z) {
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
	globalKeyStates[key] = mot_true;
	switch(key) {
		case 'W':
		case 'w':
			keyStates['w'] = mot_true;
			break;
		case 'S':
		case 's':
			keyStates['s'] = mot_true;
			break;
		case 'A':
		case 'a':
			keyStates['a'] = mot_true;
			break;
		case 'D':
		case 'd':
			keyStates['d'] = mot_true;
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
			keyStates[32] = mot_true;
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
			keyStates['w'] = mot_true;
			break;
		case GLUT_KEY_DOWN:
			keyStates['s'] = mot_true;
			break;
		case GLUT_KEY_LEFT:
			keyStates['a'] = mot_true;
			break;
		case GLUT_KEY_RIGHT:
			keyStates['d'] = mot_true;
			break;
		// Shift key
		case 112:
			isSprinting = mot_true;
			break;
	}
}

// Handles normal keys releases (arrow, function and other keys not included)
void normalKeysUpHandler (unsigned char key, int x, int y) 
{
	globalKeyStates[key] = mot_false;
	switch(key) {
		case 'W':
		case 'w':
			keyStates['w'] = mot_false;
			break;
		case 'S':
		case 's':
			keyStates['s'] = mot_false;
			break;
		case 'A':
		case 'a':
			keyStates['a'] = mot_false;
			break;
		case 'D':
		case 'd':
			keyStates['d'] = mot_false;
			break;
		// Space Key
		case 32:
			keyStates[32] = mot_false;
			break;
	}
}

// Handles arrow, function and other keys realeses (and maps them with normal keys releases)
void specialKeysUpHandler(int key, int x, int y) 
{
	switch(key) {
		case GLUT_KEY_UP:
			keyStates['w'] = mot_false;
			break;
		case GLUT_KEY_DOWN:
			keyStates['s'] = mot_false;
			break;
		case GLUT_KEY_LEFT:
			keyStates['a'] = mot_false;
			break;
		case GLUT_KEY_RIGHT:
			keyStates['d'] = mot_false;
			break;
		// Shift key
		case 112:
			isSprinting = mot_false;
			break;
	}
}

// Initializes the library
void mot_Init(GLdouble step) {
	glutKeyboardFunc(normalKeysHandler);
	glutKeyboardUpFunc(normalKeysUpHandler);
	glutSpecialFunc(specialKeysHandler);
	glutSpecialUpFunc(specialKeysUpHandler);
	glutPassiveMotionFunc(freeCameraHandler);
	glutMotionFunc(freeCameraHandler);

	// Represents the camera position
	eye = motCreatev(0, EYE_HEIGHT, 0);

	// Represents the point where the camera looks
	target = motCreatev(1, EYE_HEIGHT, 0);

	// Camerea pitch and yaw
	cameraPitch = 0;
	cameraYaw = 0; 

	// The camera speed
	speed = motCreatev(0, 0, 0);

	// The time interval between callbacks in seconds
	deltaT = step;

	// Hides the mouse cursor
	glutSetCursor(GLUT_CURSOR_NONE);
}

// Teleports the camera
mot_Vector mot_GetEyePos(void) {
	return eye;
}

// Returns the point where the camera looks
mot_Vector mot_GetTargetPos(void) {
	return target;
}

// Returns the key state for the given key
int mot_GetKeyStatus(int key) {
	return globalKeyStates[key];
}

// Returns true if he is jumping or returns false if not
int mot_GetIsJumping(void) {
	return isJumping;
}

// Returns true if he is sprinting or returns false if not
int mot_GetIsSprinting(void) {
	return isSprinting;
}

// Returns true if the game is paused or returns false if not
int mot_GetIsPaused(void) {
	return isPaused;
}

// Set if OP (over-powered). If set to true, the player can fly in any direction
void mot_SetIsOP(int state)
{
	if (state) isOP = mot_true;
	else isOP = mot_false;
}

// Change a constant. Multiple constants can be set at once: MOT_MAX_SPEED | MOT_SPRINT_MAX_SPEED
void mot_SetConstant(int constants, GLdouble value)
{
	if (constants & MOT_EYE_HEIGHT) EYE_HEIGHT = value;
	if (constants & MOT_ACCELERATION) ACCELERATION = value;
	if (constants & MOT_SPRINT_ACCELERATION) SPRINT_ACCELERATION = value;
	if (constants & MOT_BRAKE_ACCELERATION) BRAKE_ACCELERATION = value;
	if (constants & MOT_AIR_DRAG) AIR_DRAG = value;
	if (constants & MOT_MAX_SPEED) MAX_SPEED = value;
	if (constants & MOT_SPRINT_MAX_SPEED) SPRINT_MAX_SPEED = value;
	if (constants & MOT_JUMP_MAX_SPEED) JUMP_MAX_SPEED = value;
	if (constants & MOT_JUMP_SPRINT_MAX_SPEED) JUMP_SPRINT_MAX_SPEED = value;
	if (constants & MOT_JUMP_SPEED) JUMP_SPEED = value;
	if (constants & MOT_GFORCE) GFORCE = value;
}

// Get a constant
GLdouble mot_GetConstant(int constant)
{
	if (constant == MOT_EYE_HEIGHT) return EYE_HEIGHT;
	if (constant == MOT_ACCELERATION) return ACCELERATION;
	if (constant == MOT_SPRINT_ACCELERATION) return SPRINT_ACCELERATION;
	if (constant == MOT_BRAKE_ACCELERATION) return BRAKE_ACCELERATION;
	if (constant == MOT_AIR_DRAG) return AIR_DRAG;
	if (constant == MOT_MAX_SPEED) return MAX_SPEED;
	if (constant == MOT_SPRINT_MAX_SPEED) return SPRINT_MAX_SPEED;
	if (constant == MOT_JUMP_MAX_SPEED) return JUMP_MAX_SPEED;
	if (constant == MOT_JUMP_SPRINT_MAX_SPEED) return JUMP_SPRINT_MAX_SPEED;
	if (constant == MOT_JUMP_SPEED) return JUMP_SPEED;
	if (constant == MOT_GFORCE) return GFORCE;
	return 0;
}