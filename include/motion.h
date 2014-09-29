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
#define MOT_SPRINT_MAX_SPEED 7.5
// Maximum forward speed while jumping
#define MOT_JUMP_MAX_SPEED MOT_MAX_SPEED / 1.3
// Maximum forward speed while jumping and sprinting
#define MOT_JUMP_SPRINT_MAX_SPEED MOT_SPRINT_MAX_SPEED / 1.3
// The jump up speed
#define MOT_JUMP_SPEED 3.13
// Gravitational acceleration
#define MOT_GFORCE 9.81

#define MOT_PI 3.14159265359
#define MOT_DEG_TO_RAD MOT_PI / 180

// C does not support boolean
#define mot_true 1
#define mot_false 0

// Vector structure
typedef struct {
	GLdouble x, y, z;
} mot_Vector;

// Initializes the library
void mot_Init(GLdouble step);
// Main function that needs to be called in "display" function to step the player
void mot_MoveCamera(void);
// Returns the camera position
mot_Vector mot_GetEyePos(void);
// Returns the point where the camera looks
mot_Vector mot_GetTargetPos(void);
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