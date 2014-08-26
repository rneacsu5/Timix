#include <stdio.h>
#include <GL/glut.h>

#define EYE_HEIGHT 1.7
#define ACCELERATION 0.02
#define DRAG 0.01
#define AIR_DRAG 0.0025
#define MAX_SPEED 0.10
#define SPRINT_BOOST 0.07
// The formula is: GFORCE = 2 * heightOfJump / ((timeOfJump / 2) * (timeOfJump / 2 + 1))   where heigthOfJump is in meters and timeOfJump is in milliseconds
#define GFORCE 0.00583333

#define PI 3.14159265359
#define DEG_TO_RAD PI / 180
#define RAD_TO_DEG 180 / PI

// C does not support boolean
#define true 1
#define false 0

typedef struct {
	GLdouble x, y, z;
} motVector;

void motionInit(void);
void motMoveCamera(void);
motVector motGetEyePos(void);
motVector motGetTargetPos(void);
motVector motGetCamForce(void);
int motGetKeyStatus(int key);
int motGetJump(void);
int motGetSprint(void);
int motGetIsPaused(void);