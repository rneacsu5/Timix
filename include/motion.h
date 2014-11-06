/*
	motion library 
	Realistic camera movement for OpenGL
	Copyright (c) 2014 Neacsu Razvan

	Use this line before you include this header to create the implementation. USE IT ONLY ONCE!
		#define MOTION_IMPLEMENTATION

	Note: This library relies on "vector" library. You must have vector.h and must create the implementation for it. See vector.h for more info.
	Note: Everything prefixed with "motp_" or "MOTP_" (stands for "motion private") should not be used or else problems might occur.
*/

#ifndef MOTION_H_INCLUDED

#define MOTION_H_INCLUDED

#include <GL/glut.h>

#include "vector.h"
// Constants. USE THEM WITH mot_GetConstant() or mot_SetConstant()

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

// States, use them with mot_GetState() or mot_SetState()

// If the player is jumping (in mid air)
#define MOT_IS_JUMPING 1
// If the player is sprinting
#define MOT_IS_SPRINTING 1<<2
// If the game is paused
#define MOT_IS_PAUSED 1<<3
// If the player is OP (over-powered)
#define MOT_IS_OP 1<<4

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
// Teleports the camera
void mot_TeleportCamera(GLdouble x, GLdouble y, GLdouble z);
// Sets the camera (with gluLookAt())
void mot_SetCamera(void);
// Change a constant. Multiple constants can be set at once: MOT_MAX_SPEED | MOT_SPRINT_MAX_SPEED
void mot_SetConstant(int constant, GLdouble value);
// Get a constant
GLdouble mot_GetConstant(int constant);
// Get a state
int mot_GetState(int state);
// Set a state. Multiple states can be set at once.
void mot_SetState(int states, int value);
// Sets the camera velocity
void mot_SetVelocity(vect_Vector value);
// Returns the camera velocity
vect_Vector mot_GetVelocity(void);
// Sets the exit function. The default one just ends the program when ESC is pressed
void mot_ExitFunc(void(*func)(int));


// Implementation
#ifdef MOTION_IMPLEMENTATION

#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

// Constants

// Eye height
static GLdouble MOTP_EYE_HEIGHT = 1.7;
// Running acceleration
static GLdouble MOTP_ACCELERATION = 40;
// Sprinting acceleration
static GLdouble MOTP_SPRINT_ACCELERATION = 50;
// Braking acceleration
static GLdouble MOTP_BRAKE_ACCELERATION = 25;
// Air drag acceleration coefficient
static GLdouble MOTP_AIR_DRAG = 0.2;
// Maximum running speed
static GLdouble MOTP_MAX_SPEED = 5;
// Maximum sprinting speed
static GLdouble MOTP_SPRINT_MAX_SPEED = 7.5;
// Maximum forward speed while jumping
static GLdouble MOTP_JUMP_MAX_SPEED = 5 / 1.3;
// Maximum forward speed while jumping and sprinting
static GLdouble MOTP_JUMP_SPRINT_MAX_SPEED = 7.5 / 1.3;
// The jump up speed
static GLdouble MOTP_JUMP_SPEED = 3.13;
// Gravitational acceleration
static GLdouble MOTP_GFORCE = 9.81;

// Camerea pitch and yaw
static GLdouble motp_cameraYaw = 0, motp_cameraPitch = 0;
// Vectors
static vect_Vector motp_eye, motp_target, motp_velocity;
// Key states for moving and pausing the game: true if the key is pressed and false if not
static int motp_keyStates[256];
// All the key states, used with mot_GetKeyState()
static int motp_globalKeyStates[256];
// Other states
static int motp_isJumping = MOT_FALSE;
static int motp_isSprinting = MOT_FALSE;
static int motp_isPaused = MOT_FALSE;
static int motp_isOP = MOT_FALSE;

// The time interval between callbacks in seconds
static GLfloat motp_deltaT = 1 / 100.0;

// Default exit function
static void motp_defaultExitFunc(int exitCode)
{
	exit(0);
}

// Exit function
static void(*motp_exitFunc)(int) = motp_defaultExitFunc;

// Moves camera freely with no gravity
static void motp_moveOPCamera(void)
{
	// Direction in which the camera looks parallel to the ground
	vect_Vector direction = vect_Substract(vect_Create(motp_target.x, motp_eye.y, motp_target.z), motp_eye);
	vect_Normalize(&direction);

	vect_Vector velocity = vect_Create(0, 0, 0);

	if (motp_keyStates['w']) {
		velocity = vect_Add(velocity, direction);
	}
	if (motp_keyStates['s']) {
		velocity = vect_Substract(velocity, direction);
	}
	if (motp_keyStates['a']) {
		velocity = vect_Add(velocity, vect_Rotate(direction, 90, 0, 1, 0));
	}
	if (motp_keyStates['d']) {
		velocity = vect_Add(velocity, vect_Rotate(direction, -90, 0, 1, 0));
	}

	// Makes the acceleration the right length
	vect_Normalize(&velocity);
	velocity = vect_Multiply(velocity, MOTP_MAX_SPEED * 1.5);

	// Moves up or down
	if (motp_keyStates[32]) {
		velocity = vect_Add(velocity, vect_Create(0, MOTP_MAX_SPEED, 0));
	}
	if (motp_isSprinting) {
		velocity = vect_Add(velocity, vect_Create(0, -MOTP_MAX_SPEED, 0));
	}

	// Moves eye and target
	motp_eye = vect_Add(motp_eye, vect_Multiply(velocity, motp_deltaT));
	motp_target = vect_Add(motp_target, vect_Multiply(velocity, motp_deltaT));
}

// Checks for the space bar being pressed amd jumps if needed
static void motp_jumpFunc(void)
{
	// The player jumps only if the spacebar is pressed and if he is not jumping
	if (!motp_isJumping && motp_keyStates[32]) {
		motp_isJumping = MOT_TRUE;

		// Makes a push motVector based on input and direction (this allows sprint jumping)
		vect_Vector direction = vect_Substract(vect_Create(motp_target.x, motp_eye.y, motp_target.z), motp_eye);
		vect_Normalize(&direction);
		vect_Vector push = vect_Create(0, 0, 0);
		if (motp_keyStates['w']) {
			push = vect_Add(push, direction);
		}
		if (motp_keyStates['s']) {
			push = vect_Substract(push, direction);
		}
		if (motp_keyStates['a']) {
			push = vect_Add(push, vect_Rotate(direction, 90, 0, 1, 0));
		}
		if (motp_keyStates['d']) {
			push = vect_Add(push, vect_Rotate(direction, -90, 0, 1, 0));
		}

		// Makes the push motVector the right length
		vect_Normalize(&push);
		push = vect_Multiply(push, 5 * MOTP_ACCELERATION * motp_deltaT);

		// Adds the push vector to the camera velocity and limits its speed
		if (!motp_isSprinting && vect_Length(vect_Add(vect_Create(motp_velocity.x, 0, motp_velocity.z), push)) <= MOTP_JUMP_MAX_SPEED) {
			motp_velocity = vect_Add(motp_velocity, push);
		}
		else if (!motp_isSprinting && vect_Length(vect_Add(vect_Create(motp_velocity.x, 0, motp_velocity.z), push)) > MOTP_JUMP_MAX_SPEED && vect_Length(vect_Create(motp_velocity.x, 0, motp_velocity.z)) <= MOTP_JUMP_MAX_SPEED) {
			motp_velocity = vect_Add(motp_velocity, push);
			vect_Normalize(&motp_velocity);
			motp_velocity = vect_Multiply(motp_velocity, MOTP_JUMP_MAX_SPEED);
		}
		if (motp_isSprinting && vect_Length(vect_Add(vect_Create(motp_velocity.x, 0, motp_velocity.z), push)) <= MOTP_JUMP_SPRINT_MAX_SPEED)
			motp_velocity = vect_Add(motp_velocity, push);
		else if (motp_isSprinting && vect_Length(vect_Add(vect_Create(motp_velocity.x, 0, motp_velocity.z), push)) > MOTP_JUMP_SPRINT_MAX_SPEED && vect_Length(vect_Create(motp_velocity.x, 0, motp_velocity.z)) <= MOTP_JUMP_SPRINT_MAX_SPEED) {
			motp_velocity = vect_Add(motp_velocity, push);
			vect_Normalize(&motp_velocity);
			motp_velocity = vect_Multiply(motp_velocity, MOTP_JUMP_SPRINT_MAX_SPEED);
		}

		motp_velocity = vect_Add(motp_velocity, vect_Create(0, MOTP_JUMP_SPEED, 0));
	}
}

// Moves the camera one step
void mot_MoveCamera(void)
{
	// If paused
	if (motp_isPaused) {
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
		motp_isSprinting = MOT_TRUE;
	else
		motp_isSprinting = MOT_FALSE;
#endif // _WIN32

	if (motp_isOP) {
		motp_moveOPCamera();
		return;
	}

	// The following two kinematic equations are used: d = v0 * deltaT + (a * deltaT ^ 2) / 2 and v = v0 + a * deltaT
	// where v0 is the initial velocity, v is the final velocity, a is the acceleration, deltaT is the time interval and d is the distance traveled
	// Note that the previous are applied for 3D vectors (v0, v, d and a are 3D vectors)

	// Initial velocity
	vect_Vector velocity0 = motp_velocity;

	// Brake acceleration  (opposite to the velocity and parallel to the ground)
	vect_Vector drag = vect_Create(velocity0.x, 0, velocity0.z);
	vect_Normalize(&drag);
	drag = vect_Multiply(drag, -MOTP_BRAKE_ACCELERATION);
	if (!motp_isJumping) {
		if (vect_Length(motp_velocity) > vect_Length(vect_Multiply(drag, motp_deltaT))) {
			motp_velocity = vect_Add(motp_velocity, vect_Multiply(drag, motp_deltaT));
		}
		else {
			motp_velocity = vect_Create(0, 0, 0);
		}
	}

	// Checks whether to jump or not
	motp_jumpFunc();

	// Air drag acceleration (opposite to the velocity)
	vect_Vector airDrag = velocity0;
	vect_Normalize(&airDrag);
	// Air drag is proportional to the velocity
	airDrag = vect_Multiply(airDrag, -vect_Length(velocity0) * MOTP_AIR_DRAG);
	if (vect_Length(motp_velocity) > vect_Length(vect_Multiply(airDrag, motp_deltaT))) {
		motp_velocity = vect_Add(motp_velocity, vect_Multiply(airDrag, motp_deltaT));
	}
	else {
		motp_velocity = vect_Create(0, 0, 0);
	}

	// Gravity
	motp_velocity = vect_Add(motp_velocity, vect_Multiply(vect_Create(0, -MOTP_GFORCE, 0), motp_deltaT));

	// Makes the acceleration based on input and direction
	vect_Vector direction = vect_Substract(vect_Create(motp_target.x, motp_eye.y, motp_target.z), motp_eye);
	vect_Normalize(&direction);
	vect_Vector acc = vect_Create(0, 0, 0);
	if (!motp_isJumping) {
		if (motp_keyStates['w']) {
			acc = vect_Add(acc, direction);
		}
		if (motp_keyStates['s']) {
			acc = vect_Substract(acc, direction);
		}
		if (motp_keyStates['a']) {
			acc = vect_Add(acc, vect_Rotate(direction, 90, 0, 1, 0));
		}
		if (motp_keyStates['d']) {
			acc = vect_Add(acc, vect_Rotate(direction, -90, 0, 1, 0));
		}
	}

	// Makes the acceleration the right length
	vect_Normalize(&acc);
	acc = vect_Multiply(acc, MOTP_ACCELERATION);

	// If sprinting, resize the acceleration
	if (motp_isSprinting) {
		vect_Normalize(&acc);
		acc = vect_Multiply(acc, MOTP_SPRINT_ACCELERATION);
	}

	motp_velocity = vect_Add(motp_velocity, vect_Multiply(acc, motp_deltaT));

	int isMoving = motp_keyStates['w'] || motp_keyStates['a'] || motp_keyStates['s'] || motp_keyStates['d'];

	// This part limits the velocity of the camera
	if (!motp_isJumping && isMoving) {
		if (!motp_isSprinting && vect_Length(vect_Create(velocity0.x, 0, velocity0.z)) < MOTP_MAX_SPEED && vect_Length(vect_Create(motp_velocity.x, 0, motp_velocity.z)) > MOTP_MAX_SPEED) {
			vect_Vector s = vect_Create(motp_velocity.x, 0, motp_velocity.z);
			vect_Normalize(&s);
			s = vect_Multiply(s, MOTP_MAX_SPEED);
			motp_velocity = vect_Create(s.x, motp_velocity.y, s.z);
		}
		else if (!motp_isSprinting && vect_Length(vect_Create(velocity0.x, 0, velocity0.z)) >= MOTP_MAX_SPEED) {
			vect_Vector velocity1 = velocity0;
			velocity1 = vect_Add(velocity1, vect_Multiply(drag, motp_deltaT));
			velocity1 = vect_Add(velocity1, vect_Multiply(airDrag, motp_deltaT));
			GLdouble length = vect_Length(velocity1);
			vect_Normalize(&motp_velocity);
			motp_velocity = vect_Multiply(motp_velocity, length);
			if (vect_Length(vect_Create(motp_velocity.x, 0, motp_velocity.z)) < MOTP_MAX_SPEED) {
				vect_Vector s = vect_Create(motp_velocity.x, 0, motp_velocity.z);
				vect_Normalize(&s);
				s = vect_Multiply(s, MOTP_MAX_SPEED);
				motp_velocity = vect_Create(s.x, motp_velocity.y, s.z);
			}
		}
		if (motp_isSprinting && vect_Length(vect_Create(velocity0.x, 0, velocity0.z)) < MOTP_SPRINT_MAX_SPEED && vect_Length(vect_Create(motp_velocity.x, 0, motp_velocity.z)) > MOTP_SPRINT_MAX_SPEED) {
			vect_Vector s = vect_Create(motp_velocity.x, 0, motp_velocity.z);
			vect_Normalize(&s);
			s = vect_Multiply(s, MOTP_SPRINT_MAX_SPEED);
			motp_velocity = vect_Create(s.x, motp_velocity.y, s.z);
		}
		else if (motp_isSprinting && vect_Length(vect_Create(velocity0.x, 0, velocity0.z)) >= MOTP_SPRINT_MAX_SPEED) {
			vect_Vector velocity1 = velocity0;
			velocity1 = vect_Add(velocity1, vect_Multiply(drag, motp_deltaT));
			velocity1 = vect_Add(velocity1, vect_Multiply(airDrag, motp_deltaT));
			GLdouble length = vect_Length(velocity1);
			vect_Normalize(&motp_velocity);
			motp_velocity = vect_Multiply(motp_velocity, length);
			if (vect_Length(vect_Create(motp_velocity.x, 0, motp_velocity.z)) < MOTP_SPRINT_MAX_SPEED) {
				vect_Vector s = vect_Create(motp_velocity.x, 0, motp_velocity.z);
				vect_Normalize(&s);
				s = vect_Multiply(s, MOTP_SPRINT_MAX_SPEED);
				motp_velocity = vect_Create(s.x, motp_velocity.y, s.z);
			}
		}
	}

	// The distance traveled
	vect_Vector d = vect_Add(vect_Multiply(velocity0, motp_deltaT), vect_Multiply(vect_Substract(motp_velocity, velocity0), motp_deltaT / 2.0));

	// Moves eye and target
	motp_eye = vect_Add(motp_eye, d);
	motp_target = vect_Add(motp_target, d);

	// Check for floor
	if (motp_eye.y < MOTP_EYE_HEIGHT) {
		motp_target.y += MOTP_EYE_HEIGHT - motp_eye.y;
		motp_eye.y = MOTP_EYE_HEIGHT;
		motp_isJumping = MOT_FALSE;
		motp_velocity = vect_Create(motp_velocity.x, 0, motp_velocity.z);
	}
}

// Handles mouse movements
static void motp_freeCameraHandler(int x, int y)
{
	// If paused, don't do anything
	if (motp_isPaused) {
		return;
	}

	GLint v[4];
	glGetIntegerv(GL_VIEWPORT, v); // Gets 4 values: X, Y position of window, Width and Heigth of viewport
	// Determines the angle on each axis based on mouse position
	motp_cameraYaw += -45 + 90 * x / (GLdouble)v[2];
	motp_cameraPitch += -30 + 60 * y / (GLdouble)v[3];

	// cameraYaw must not exeed 360 or be below -360 degrees
	motp_cameraYaw -= ((int)motp_cameraYaw / 360) * 360;

	// cameraPitch must not exeed 89 degrees or be below -89 degrees
	if (motp_cameraPitch > 89) {
		motp_cameraPitch = 89;
	}
	if (motp_cameraPitch < -89) {
		motp_cameraPitch = -89;
	}

	// Some notations
	GLdouble sinY = sin(-motp_cameraPitch * MOT_DEG_TO_RAD);
	GLdouble sinX = sin(motp_cameraYaw * MOT_DEG_TO_RAD);
	GLdouble cosX = cos(motp_cameraYaw * MOT_DEG_TO_RAD);

	// Makes the target vector based on cameraYaw and cameraPitch (rotating the vector using motRotatev() is a bit buggy)
	motp_target = vect_Create(cosX, 0, sinX);
	motp_target = vect_Multiply(motp_target, sqrt(1 - sinY * sinY));
	motp_target = vect_Add(motp_target, vect_Create(0, sinY, 0));

	// Adds the eye position so that the camera points to the right place
	motp_target = vect_Add(motp_target, motp_eye);
}

// Handles normal key presses (arrow, function and other keys not included)
static void motp_normalKeysHandler(unsigned char key, int x, int y)
{
	motp_globalKeyStates[key] = MOT_TRUE;
	switch (key) {
	case 'W':
	case 'w':
		motp_keyStates['w'] = MOT_TRUE;
		break;
	case 'S':
	case 's':
		motp_keyStates['s'] = MOT_TRUE;
		break;
	case 'A':
	case 'a':
		motp_keyStates['a'] = MOT_TRUE;
		break;
	case 'D':
	case 'd':
		motp_keyStates['d'] = MOT_TRUE;
		break;
	case 'P':
	case 'p':
		motp_isPaused = !motp_isPaused;
		if (motp_isPaused) {
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
		motp_keyStates[32] = MOT_TRUE;
		break;
		// Esc Key
	case 27:
		motp_exitFunc(0);
		break;
	}
}

// Handles arrow, function and other keys presses (and maps them with normal keys)
static void motp_specialKeysHandler(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		motp_keyStates['w'] = MOT_TRUE;
		break;
	case GLUT_KEY_DOWN:
		motp_keyStates['s'] = MOT_TRUE;
		break;
	case GLUT_KEY_LEFT:
		motp_keyStates['a'] = MOT_TRUE;
		break;
	case GLUT_KEY_RIGHT:
		motp_keyStates['d'] = MOT_TRUE;
		break;
		// Shift key
	case 112:
		motp_isSprinting = MOT_TRUE;
		break;
	}
}

// Handles normal keys releases (arrow, function and other keys not included)
static void motp_normalKeysUpHandler(unsigned char key, int x, int y)
{
	motp_globalKeyStates[key] = MOT_FALSE;
	switch (key) {
	case 'W':
	case 'w':
		motp_keyStates['w'] = MOT_FALSE;
		break;
	case 'S':
	case 's':
		motp_keyStates['s'] = MOT_FALSE;
		break;
	case 'A':
	case 'a':
		motp_keyStates['a'] = MOT_FALSE;
		break;
	case 'D':
	case 'd':
		motp_keyStates['d'] = MOT_FALSE;
		break;
		// Space Key
	case 32:
		motp_keyStates[32] = MOT_FALSE;
		break;
	}
}

// Handles arrow, function and other keys realeses (and maps them with normal keys releases)
static void motp_specialKeysUpHandler(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		motp_keyStates['w'] = MOT_FALSE;
		break;
	case GLUT_KEY_DOWN:
		motp_keyStates['s'] = MOT_FALSE;
		break;
	case GLUT_KEY_LEFT:
		motp_keyStates['a'] = MOT_FALSE;
		break;
	case GLUT_KEY_RIGHT:
		motp_keyStates['d'] = MOT_FALSE;
		break;
		// Shift key
	case 112:
		motp_isSprinting = MOT_FALSE;
		break;
	}
}

// Initializes the library
void mot_Init(GLdouble step)
{
	glutKeyboardFunc(motp_normalKeysHandler);
	glutKeyboardUpFunc(motp_normalKeysUpHandler);
	glutSpecialFunc(motp_specialKeysHandler);
	glutSpecialUpFunc(motp_specialKeysUpHandler);
	glutPassiveMotionFunc(motp_freeCameraHandler);
	glutMotionFunc(motp_freeCameraHandler);

	// Represents the camera position
	motp_eye = vect_Create(0, MOTP_EYE_HEIGHT, 0);

	// Represents the point where the camera looks
	motp_target = vect_Create(1, MOTP_EYE_HEIGHT, 0);

	// Camerea pitch and yaw
	motp_cameraPitch = 0;
	motp_cameraYaw = 0;

	// The camera velocity
	motp_velocity = vect_Create(0, 0, 0);

	// The time interval between callbacks in seconds
	motp_deltaT = step;

	// Hides the mouse cursor
	glutSetCursor(GLUT_CURSOR_NONE);
}

// Teleports the camera
vect_Vector mot_GetEyePos(void)
{
	return motp_eye;
}

// Returns the point where the camera looks
vect_Vector mot_GetTargetPos(void)
{
	return motp_target;
}

// Teleports the camera
void mot_TeleportCamera(GLdouble x, GLdouble y, GLdouble z)
{
	motp_target.x -= motp_eye.x - x;
	motp_target.y -= motp_eye.y - y;
	motp_target.z -= motp_eye.z - z;
	motp_eye.x = x;
	motp_eye.y = y;
	motp_eye.z = z;
}

// Sets the camera (with gluLookAt())
void mot_SetCamera(void)
{
	gluLookAt(motp_eye.x, motp_eye.y, motp_eye.z, motp_target.x, motp_target.y, motp_target.z, 0, 1, 0);
}

// Returns the key state for the given key
int mot_GetKeyStatus(int key)
{
	return motp_globalKeyStates[key];
}

// Change a constant. Multiple constants can be set at once: MOT_MAX_SPEED | MOT_SPRINT_MAX_SPEED
void mot_SetConstant(int constants, GLdouble value)
{
	if (constants & MOT_EYE_HEIGHT) MOTP_EYE_HEIGHT = value;
	if (constants & MOT_ACCELERATION) MOTP_ACCELERATION = value;
	if (constants & MOT_SPRINT_ACCELERATION) MOTP_SPRINT_ACCELERATION = value;
	if (constants & MOT_BRAKE_ACCELERATION) MOTP_BRAKE_ACCELERATION = value;
	if (constants & MOT_AIR_DRAG) MOTP_AIR_DRAG = value;
	if (constants & MOT_MAX_SPEED) MOTP_MAX_SPEED = value;
	if (constants & MOT_SPRINT_MAX_SPEED) MOTP_SPRINT_MAX_SPEED = value;
	if (constants & MOT_JUMP_MAX_SPEED) MOTP_JUMP_MAX_SPEED = value;
	if (constants & MOT_JUMP_SPRINT_MAX_SPEED) MOTP_JUMP_SPRINT_MAX_SPEED = value;
	if (constants & MOT_JUMP_SPEED) MOTP_JUMP_SPEED = value;
	if (constants & MOT_GFORCE) MOTP_GFORCE = value;
}

// Get a constant
GLdouble mot_GetConstant(int constant)
{
	if (constant == MOT_EYE_HEIGHT) return MOTP_EYE_HEIGHT;
	if (constant == MOT_ACCELERATION) return MOTP_ACCELERATION;
	if (constant == MOT_SPRINT_ACCELERATION) return MOTP_SPRINT_ACCELERATION;
	if (constant == MOT_BRAKE_ACCELERATION) return MOTP_BRAKE_ACCELERATION;
	if (constant == MOT_AIR_DRAG) return MOTP_AIR_DRAG;
	if (constant == MOT_MAX_SPEED) return MOTP_MAX_SPEED;
	if (constant == MOT_SPRINT_MAX_SPEED) return MOTP_SPRINT_MAX_SPEED;
	if (constant == MOT_JUMP_MAX_SPEED) return MOTP_JUMP_MAX_SPEED;
	if (constant == MOT_JUMP_SPRINT_MAX_SPEED) return MOTP_JUMP_SPRINT_MAX_SPEED;
	if (constant == MOT_JUMP_SPEED) return MOTP_JUMP_SPEED;
	if (constant == MOT_GFORCE) return MOTP_GFORCE;
	return 0;
}

// Get a state
int mot_GetState(int state)
{
	if (state == MOT_IS_JUMPING) return motp_isJumping;
	if (state == MOT_IS_SPRINTING) return motp_isSprinting;
	if (state == MOT_IS_PAUSED) return motp_isPaused;
	if (state == MOT_IS_OP) return motp_isOP;
	return 0;
}

// Set a state. Multiple states can be set at once.
void mot_SetState(int states, int value)
{
	if (states & MOT_IS_JUMPING) motp_isJumping = (value) ? MOT_TRUE : MOT_FALSE;
	if (states & MOT_IS_SPRINTING) motp_isSprinting = (value) ? MOT_TRUE : MOT_FALSE;
	if (states & MOT_IS_PAUSED) motp_isPaused = (value) ? MOT_TRUE : MOT_FALSE;
	if (states & MOT_IS_OP) motp_isOP = (value) ? MOT_TRUE : MOT_FALSE;
}

// Sets the camera velocity
void mot_SetVelocity(vect_Vector value)
{
	motp_velocity = value;
}

// Returns the camera velocity
vect_Vector mot_GetVelocity(void)
{
	return motp_velocity;
}

// Sets the exit function. The default one just ends the program when ESC is pressed
void mot_ExitFunc(void(*func)(int))
{
	motp_exitFunc = func;
}

#endif // MOTION_IMPLEMENTATION

#endif // MOTION_H_INCLUDED