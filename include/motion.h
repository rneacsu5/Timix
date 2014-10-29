/*
	motion library 
	Realistic camera movement for OpenGL
	Copyright (c) 2014 Neacsu Razvan

	Use this line before you include this header to create the implementation. USE IT ONLY ONCE!
		#define MOTION_IMPLEMENTATION

	Note that this library relies on "vector" library. You must have vector.h and must create the implementation for it. See vector.h for more info.
*/

#ifndef MOTION_H

#define MOTION_H

#include <GL/glut.h>

#include "vector.h"
// Constants. USE THEM WITH mot_GetConstant()

// Eye height
#define MOT_EYE_HEIGHT 1
// Running acceleration
#define MOT_ACCELERATION 1<<1
// Sprinting acceleration
#define MOT_SPRINT_ACCELERATION 1<<2
// Braking acceleration
#define MOT_BRAKE_ACCELERATION 1<<3
// Air drag acceleration coefficient
#define MOT_AIR_DRAG 1<<4
// Maximum running speed
#define MOT_MAX_SPEED 1<<5
// Maximum sprinting speed
#define MOT_SPRINT_MAX_SPEED 1<<6
// Maximum forward speed while jumping
#define MOT_JUMP_MAX_SPEED 1<<7
// Maximum forward speed while jumping and sprinting
#define MOT_JUMP_SPRINT_MAX_SPEED 1<<8
// The jump up speed
#define MOT_JUMP_SPEED 1<<9
// Gravitational acceleration
#define MOT_GFORCE 1<<10

#define MOT_PI 3.14159265359
#define MOT_DEG_TO_RAD MOT_PI / 180

// C does not support boolean
#define MOT_TRUE 1
#define MOT_FALSE 0


// Initializes the library
void mot_Init(GLdouble step);
// Main function that needs to be called in "display" function to step the player
void mot_MoveCamera(void);
// Returns the camera position
vect_Vector mot_GetEyePos(void);
// Returns the point where the camera looks
vect_Vector mot_GetTargetPos(void);
// Returns the key state for the given key
int mot_GetKeyStatus(int key);
// Returns true if he is jumping or returns false if not
int mot_GetIsJumping(void);
// Returns true if he is sprinting or returns false if not
int mot_GetIsSprinting(void);
// Returns true if the game is paused or returns false if not
int mot_GetIsPaused(void);
// Teleports the camera
void mot_TeleportCamera(GLdouble x, GLdouble y, GLdouble z);
// Set if OP (over-powered). If set to true, the player can fly in any direction
void mot_SetIsOP(int state);
// Change a constant. Multiple constants can be set at once: MOT_MAX_SPEED | MOT_SPRINT_MAX_SPEED
void mot_SetConstant(int constant, GLdouble value);
// Get a constant
GLdouble mot_GetConstant(int constant);


//Implementation
#ifdef MOTION_IMPLEMENTATION

#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

// Constants

// Eye height
static GLdouble EYE_HEIGHT = 1.7;
// Running acceleration
static GLdouble ACCELERATION = 40;
// Sprinting acceleration
static GLdouble SPRINT_ACCELERATION = 50;
// Braking acceleration
static GLdouble BRAKE_ACCELERATION = 25;
// Air drag acceleration coefficient
static GLdouble AIR_DRAG = 0.2;
// Maximum running speed
static GLdouble MAX_SPEED = 5;
// Maximum sprinting speed
static GLdouble SPRINT_MAX_SPEED = 7.5;
// Maximum forward speed while jumping
static GLdouble JUMP_MAX_SPEED = 5 / 1.3;
// Maximum forward speed while jumping and sprinting
static GLdouble JUMP_SPRINT_MAX_SPEED = 7.5 / 1.3;
// The jump up speed
static GLdouble JUMP_SPEED = 3.13;
// Gravitational acceleration
static GLdouble GFORCE = 9.81;

// Camerea pitch and yaw
static GLdouble cameraYaw = 0, cameraPitch = 0;
// Vectors
static vect_Vector eye, target, speed;
// Key states for moving and pausing the game: true if the key is pressed and false if not
static int keyStates[256];
// All the key states, used with mot_GetKeyState()
static int globalKeyStates[256];
// Other states
static int isJumping = MOT_FALSE;
static int isSprinting = MOT_FALSE;
static int isPaused = MOT_FALSE;
static int isOP = MOT_FALSE;

// The time interval between callbacks in seconds
static GLfloat deltaT = 1 / 100.0;

// Moves camera freely with no gravity
static void moveOPCamera(void)
{
	// Moves mouse to the middle
	GLint v[4];
	glGetIntegerv(GL_VIEWPORT, v);
	glutWarpPointer(v[2] / 2, v[3] / 2);

	// Direction in which the camera looks parallel to the ground
	vect_Vector direction = vect_Substract(vect_Create(target.x, eye.y, target.z), eye);
	vect_Normalize(&direction);

	vect_Vector speed = vect_Create(0, 0, 0);

	if (keyStates['w']) {
		speed = vect_Add(speed, direction);
	}
	if (keyStates['s']) {
		speed = vect_Substract(speed, direction);
	}
	if (keyStates['a']) {
		speed = vect_Add(speed, vect_Rotate(direction, 90, 0, 1, 0));
	}
	if (keyStates['d']) {
		speed = vect_Add(speed, vect_Rotate(direction, -90, 0, 1, 0));
	}

	// Makes the acceleration the right length
	vect_Normalize(&speed);
	speed = vect_Multiply(speed, MAX_SPEED * 1.5);

	// Moves up or down
	if (keyStates[32]) {
		speed = vect_Add(speed, vect_Create(0, MAX_SPEED, 0));
	}
	if (isSprinting) {
		speed = vect_Add(speed, vect_Create(0, -MAX_SPEED, 0));
	}


	// Moves eye and target
	eye = vect_Add(eye, vect_Multiply(speed, deltaT));
	target = vect_Add(target, vect_Multiply(speed, deltaT));
	gluLookAt(eye.x, eye.y, eye.z, target.x, target.y, target.z, 0, 1, 0);
}

// Checks for the space bar being pressed amd jumps if needed
static void jumpFunc(void)
{
	// The player jumps only if the spacebar is pressed and if he is not jumping
	if (!isJumping && keyStates[32]) {
		isJumping = MOT_TRUE;

		// Makes a push motVector based on input and direction (this allows sprint jumping)
		vect_Vector direction = vect_Substract(vect_Create(target.x, eye.y, target.z), eye);
		vect_Normalize(&direction);
		vect_Vector push = vect_Create(0, 0, 0);
		if (keyStates['w']) {
			push = vect_Add(push, direction);
		}
		if (keyStates['s']) {
			push = vect_Substract(push, direction);
		}
		if (keyStates['a']) {
			push = vect_Add(push, vect_Rotate(direction, 90, 0, 1, 0));
		}
		if (keyStates['d']) {
			push = vect_Add(push, vect_Rotate(direction, -90, 0, 1, 0));
		}

		// Makes the push motVector the right length
		vect_Normalize(&push);
		push = vect_Multiply(push, 5 * ACCELERATION * deltaT);

		// Adds the push motVector to the camera speed and limits its speed
		if (!isSprinting && vect_Length(vect_Add(vect_Create(speed.x, 0, speed.z), push)) <= JUMP_MAX_SPEED) {
			speed = vect_Add(speed, push);
		}
		else if (!isSprinting && vect_Length(vect_Add(vect_Create(speed.x, 0, speed.z), push)) > JUMP_MAX_SPEED && vect_Length(vect_Create(speed.x, 0, speed.z)) <= JUMP_MAX_SPEED) {
			speed = vect_Add(speed, push);
			vect_Normalize(&speed);
			speed = vect_Multiply(speed, JUMP_MAX_SPEED);
		}
		if (isSprinting && vect_Length(vect_Add(vect_Create(speed.x, 0, speed.z), push)) <= JUMP_SPRINT_MAX_SPEED)
			speed = vect_Add(speed, push);
		else if (isSprinting && vect_Length(vect_Add(vect_Create(speed.x, 0, speed.z), push)) > JUMP_SPRINT_MAX_SPEED && vect_Length(vect_Create(speed.x, 0, speed.z)) <= JUMP_SPRINT_MAX_SPEED) {
			speed = vect_Add(speed, push);
			vect_Normalize(&speed);
			speed = vect_Multiply(speed, JUMP_SPRINT_MAX_SPEED);
		}

		speed = vect_Add(speed, vect_Create(0, JUMP_SPEED, 0));
	}
}

// Moves the camera one step
void mot_MoveCamera(void)
{
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
		isSprinting = MOT_TRUE;
	else
		isSprinting = MOT_FALSE;
#endif // _WIN32

	if (isOP) {
		moveOPCamera();
		return;
	}

	// The following two kinematic equations are used: d = v0 * deltaT + (a * deltaT ^ 2) / 2 and v = v0 + a * deltaT
	// where v0 is the initial speed, v is the final speed, a is the acceleration, deltaT is the time interval and d is the distance traveled
	// Note that the previous are applied for 3D vectors (v0, v, d and a are 3D vectors)

	// Initial speed
	vect_Vector speed0 = speed;

	// Brake acceleration  (opposite to the speed and parallel to the ground)
	vect_Vector drag = vect_Create(speed0.x, 0, speed0.z);
	vect_Normalize(&drag);
	drag = vect_Multiply(drag, -BRAKE_ACCELERATION);
	if (!isJumping) {
		if (vect_Length(speed) > vect_Length(vect_Multiply(drag, deltaT))) {
			speed = vect_Add(speed, vect_Multiply(drag, deltaT));
		}
		else {
			speed = vect_Create(0, 0, 0);
		}
	}

	// Checks whether to jump or not
	jumpFunc();

	// Air drag acceleration (opposite to the speed)
	vect_Vector airDrag = speed0;
	vect_Normalize(&airDrag);
	// Air drag is proportional to the speed
	airDrag = vect_Multiply(airDrag, -vect_Length(speed0) * AIR_DRAG);
	if (vect_Length(speed) > vect_Length(vect_Multiply(airDrag, deltaT))) {
		speed = vect_Add(speed, vect_Multiply(airDrag, deltaT));
	}
	else {
		speed = vect_Create(0, 0, 0);
	}

	// Gravity
	speed = vect_Add(speed, vect_Multiply(vect_Create(0, -GFORCE, 0), deltaT));

	// Makes the acceleration based on input and direction
	vect_Vector direction = vect_Substract(vect_Create(target.x, eye.y, target.z), eye);
	vect_Normalize(&direction);
	vect_Vector acc = vect_Create(0, 0, 0);
	if (!isJumping) {
		if (keyStates['w']) {
			acc = vect_Add(acc, direction);
		}
		if (keyStates['s']) {
			acc = vect_Substract(acc, direction);
		}
		if (keyStates['a']) {
			acc = vect_Add(acc, vect_Rotate(direction, 90, 0, 1, 0));
		}
		if (keyStates['d']) {
			acc = vect_Add(acc, vect_Rotate(direction, -90, 0, 1, 0));
		}
	}

	// Makes the acceleration the right length
	vect_Normalize(&acc);
	acc = vect_Multiply(acc, ACCELERATION);

	// If sprinting, resize the acceleration
	if (isSprinting) {
		vect_Normalize(&acc);
		acc = vect_Multiply(acc, SPRINT_ACCELERATION);
	}

	speed = vect_Add(speed, vect_Multiply(acc, deltaT));

	int isMoving = keyStates['w'] || keyStates['a'] || keyStates['s'] || keyStates['d'];

	// This part limits the speed of the camera
	if (!isJumping && isMoving) {
		if (!isSprinting && vect_Length(vect_Create(speed0.x, 0, speed0.z)) < MAX_SPEED && vect_Length(vect_Create(speed.x, 0, speed.z)) > MAX_SPEED) {
			vect_Vector s = vect_Create(speed.x, 0, speed.z);
			vect_Normalize(&s);
			s = vect_Multiply(s, MAX_SPEED);
			speed = vect_Create(s.x, speed.y, s.z);
		}
		else if (!isSprinting && vect_Length(vect_Create(speed0.x, 0, speed0.z)) >= MAX_SPEED) {
			vect_Vector speed1 = speed0;
			speed1 = vect_Add(speed1, vect_Multiply(drag, deltaT));
			speed1 = vect_Add(speed1, vect_Multiply(airDrag, deltaT));
			GLdouble length = vect_Length(speed1);
			vect_Normalize(&speed);
			speed = vect_Multiply(speed, length);
			if (vect_Length(vect_Create(speed.x, 0, speed.z)) < MAX_SPEED) {
				vect_Vector s = vect_Create(speed.x, 0, speed.z);
				vect_Normalize(&s);
				s = vect_Multiply(s, MAX_SPEED);
				speed = vect_Create(s.x, speed.y, s.z);
			}
		}
		if (isSprinting && vect_Length(vect_Create(speed0.x, 0, speed0.z)) < SPRINT_MAX_SPEED && vect_Length(vect_Create(speed.x, 0, speed.z)) > SPRINT_MAX_SPEED) {
			vect_Vector s = vect_Create(speed.x, 0, speed.z);
			vect_Normalize(&s);
			s = vect_Multiply(s, SPRINT_MAX_SPEED);
			speed = vect_Create(s.x, speed.y, s.z);
		}
		else if (isSprinting && vect_Length(vect_Create(speed0.x, 0, speed0.z)) >= SPRINT_MAX_SPEED) {
			vect_Vector speed1 = speed0;
			speed1 = vect_Add(speed1, vect_Multiply(drag, deltaT));
			speed1 = vect_Add(speed1, vect_Multiply(airDrag, deltaT));
			GLdouble length = vect_Length(speed1);
			vect_Normalize(&speed);
			speed = vect_Multiply(speed, length);
			if (vect_Length(vect_Create(speed.x, 0, speed.z)) < SPRINT_MAX_SPEED) {
				vect_Vector s = vect_Create(speed.x, 0, speed.z);
				vect_Normalize(&s);
				s = vect_Multiply(s, SPRINT_MAX_SPEED);
				speed = vect_Create(s.x, speed.y, s.z);
			}
		}
	}

	// The distance traveled
	vect_Vector d = vect_Add(vect_Multiply(speed0, deltaT), vect_Multiply(vect_Substract(speed, speed0), deltaT / 2.0));

	// Moves eye and target
	eye = vect_Add(eye, d);
	target = vect_Add(target, d);

	// Check for floor
	if (eye.y < EYE_HEIGHT) {
		target.y += EYE_HEIGHT - eye.y;
		eye.y = EYE_HEIGHT;
		isJumping = MOT_FALSE;
		speed = vect_Create(speed.x, 0, speed.z);
	}

	gluLookAt(eye.x, eye.y, eye.z, target.x, target.y, target.z, 0, 1, 0);
}

// Handels mouse movements
static void freeCameraHandler(int x, int y)
{
	// If paused, don't do anything
	if (isPaused) {
		return;
	}

	GLint v[4];
	glGetIntegerv(GL_VIEWPORT, v); // Gets 4 values: X, Y position of window, Width and Heigth of viewport
	// Determines the angle on each axis based on mouse position
	cameraYaw += -45 + 90 * x / (GLdouble)v[2];
	cameraPitch += -30 + 60 * y / (GLdouble)v[3];

	// cameraYaw must not exeed 360 or be below -360 degrees
	cameraYaw -= ((int)cameraYaw / 360) * 360;

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
	target = vect_Create(cosX, 0, sinX);
	target = vect_Multiply(target, sqrt(1 - sinY * sinY));
	target = vect_Add(target, vect_Create(0, sinY, 0));

	// Adds the eye position so that the camera points to the right place
	target = vect_Add(target, eye);
}

// Teleports the camera
void mot_TeleportCamera(GLdouble x, GLdouble y, GLdouble z)
{
	target.x -= eye.x - x;
	target.y -= eye.y - y;
	target.z -= eye.z - z;
	eye.x = x;
	eye.y = y;
	eye.z = z;
}

// Handles normal key presses (arrow, function and other keys not included)
static void normalKeysHandler(unsigned char key, int x, int y)
{
	globalKeyStates[key] = MOT_TRUE;
	switch (key) {
	case 'W':
	case 'w':
		keyStates['w'] = MOT_TRUE;
		break;
	case 'S':
	case 's':
		keyStates['s'] = MOT_TRUE;
		break;
	case 'A':
	case 'a':
		keyStates['a'] = MOT_TRUE;
		break;
	case 'D':
	case 'd':
		keyStates['d'] = MOT_TRUE;
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
		keyStates[32] = MOT_TRUE;
		break;
		// Esc Key
	case 27:
		exit(0);
		break;
	}
}

// Handles arrow, function and other keys presses (and maps them with normal keys)
static void specialKeysHandler(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		keyStates['w'] = MOT_TRUE;
		break;
	case GLUT_KEY_DOWN:
		keyStates['s'] = MOT_TRUE;
		break;
	case GLUT_KEY_LEFT:
		keyStates['a'] = MOT_TRUE;
		break;
	case GLUT_KEY_RIGHT:
		keyStates['d'] = MOT_TRUE;
		break;
		// Shift key
	case 112:
		isSprinting = MOT_TRUE;
		break;
	}
}

// Handles normal keys releases (arrow, function and other keys not included)
static void normalKeysUpHandler(unsigned char key, int x, int y)
{
	globalKeyStates[key] = MOT_FALSE;
	switch (key) {
	case 'W':
	case 'w':
		keyStates['w'] = MOT_FALSE;
		break;
	case 'S':
	case 's':
		keyStates['s'] = MOT_FALSE;
		break;
	case 'A':
	case 'a':
		keyStates['a'] = MOT_FALSE;
		break;
	case 'D':
	case 'd':
		keyStates['d'] = MOT_FALSE;
		break;
		// Space Key
	case 32:
		keyStates[32] = MOT_FALSE;
		break;
	}
}

// Handles arrow, function and other keys realeses (and maps them with normal keys releases)
static void specialKeysUpHandler(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		keyStates['w'] = MOT_FALSE;
		break;
	case GLUT_KEY_DOWN:
		keyStates['s'] = MOT_FALSE;
		break;
	case GLUT_KEY_LEFT:
		keyStates['a'] = MOT_FALSE;
		break;
	case GLUT_KEY_RIGHT:
		keyStates['d'] = MOT_FALSE;
		break;
		// Shift key
	case 112:
		isSprinting = MOT_FALSE;
		break;
	}
}

// Initializes the library
void mot_Init(GLdouble step)
{
	glutKeyboardFunc(normalKeysHandler);
	glutKeyboardUpFunc(normalKeysUpHandler);
	glutSpecialFunc(specialKeysHandler);
	glutSpecialUpFunc(specialKeysUpHandler);
	glutPassiveMotionFunc(freeCameraHandler);
	glutMotionFunc(freeCameraHandler);

	// Represents the camera position
	eye = vect_Create(0, EYE_HEIGHT, 0);

	// Represents the point where the camera looks
	target = vect_Create(1, EYE_HEIGHT, 0);

	// Camerea pitch and yaw
	cameraPitch = 0;
	cameraYaw = 0;

	// The camera speed
	speed = vect_Create(0, 0, 0);

	// The time interval between callbacks in seconds
	deltaT = step;

	// Hides the mouse cursor
	glutSetCursor(GLUT_CURSOR_NONE);
}

// Teleports the camera
vect_Vector mot_GetEyePos(void)
{
	return eye;
}

// Returns the point where the camera looks
vect_Vector mot_GetTargetPos(void)
{
	return target;
}

// Returns the key state for the given key
int mot_GetKeyStatus(int key)
{
	return globalKeyStates[key];
}

// Returns true if he is jumping or returns false if not
int mot_GetIsJumping(void)
{
	return isJumping;
}

// Returns true if he is sprinting or returns false if not
int mot_GetIsSprinting(void)
{
	return isSprinting;
}

// Returns true if the game is paused or returns false if not
int mot_GetIsPaused(void)
{
	return isPaused;
}

// Set if OP (over-powered). If set to true, the player can fly in any direction
void mot_SetIsOP(int state)
{
	if (state) isOP = MOT_TRUE;
	else isOP = MOT_FALSE;
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

#endif // MOTION_IMPLEMENTATION

#endif // MOTION_H