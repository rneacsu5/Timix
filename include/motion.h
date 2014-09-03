/*
	motion library 
	Realistic camera movement for OpenGL
	Copyright (c) 2014 Neacsu Razvan
*/

#include <stdio.h>
#include <GL/glut.h>

// Eye height
#define MOT_EYE_HEIGHT 1.7
// Running acceleration
#define MOT_ACCELERATION 40
// Sprinting acceleration
#define MOT_SPRINT_ACCELERATION 50
// Braking acceleration
#define MOT_BRAKE_ACCELERATION 25
// Air drag acceleration coefficient
#define MOT_AIR_DRAG 0.2
// Maximum running speed
#define MOT_MAX_SPEED 5
// Maximum sprinting speed
#define MOT_SPRINT_MAX_SPEED 10
// Maximum forward speed while jumping
#define MOT_JUMP_MAX_SPEED MOT_MAX_SPEED / 1.3
// Maximum forward speed while jumping and sprinting
#define MOT_JUMP_SPRINT_MAX_SPEED MOT_SPRINT_MAX_SPEED / 1.3
// The jump up speed
#define MOT_JUMP_SPEED 3.13
// Gravitational acceleration
#define MOT_GFORCE 9.8

#define MOT_PI 3.14159265359
#define MOT_DEG_TO_RAD MOT_PI / 180

// C does not support boolean
#define true 1
#define false 0

// Vector structure
typedef struct {
	GLdouble x, y, z;
} motVector;

// Initializes the library
void motionInit(void);
// Main function that needs to be called in "display" function
void motMoveCamera(void);
// Returns the camera position
motVector motGetEyePos(void);
// Returns the point where the camera looks
motVector motGetTargetPos(void);
// Returns the key state for the given key
int motGetKeyStatus(int key);
// Returns true if he is jumping or returns false if not
int motGetIsJumping(void);
// Returns true if he is sprinting or returns false if not
int motGetIsSprinting(void);
// Returns true if the game is paused or returns false if not
int motGetIsPaused(void);
// Teleports the camera
void motTeleportCamera(GLdouble x, GLdouble y, GLdouble z);