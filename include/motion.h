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

// C does not support boolean
#define true 1
#define false 0

void motionInit(void);
void motMoveCamera(void);
vector motGetEyePos(void);
vector motGetTargetPos(void);
vector motGetCamForce(void);
int motGetKeyStatus(int key);
int motGetJump(void);
int motGetSprint(void);