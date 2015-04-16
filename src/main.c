#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include <bass.h>


#include "../include/utility.h"
#define COLORTEXT_IMPLEMENTATION
#include "../include/colortext.h"
#define LOBJDER_IMPLEMENTATION
#define LOBJDER_COLOR_OUTPUT
#include "../include/lobjder.h"
#define VECTOR_IMPLEMENTATION
#include "../include/vector.h"
#define MOTION_IMPLEMENTATION
#include "../include/motion.h"

// C does not support boolean
#define true 1
#define false 0

// Light parameters
GLfloat lightPos[] = {10, 4.5, 10, 1};
GLfloat lightAmbient[] = {0.5, 0.5, 0.5, 1};
GLfloat lightDiffuse[] = {0.8, 0.8, 0.8, 1};
GLfloat lightSpecular[] = {0.4, 0.4, 0.4, 1};

GLfloat fogColor[] = {0.8, 0.8, 0.8, 1.0};

GLdouble a1 = 0, a2 = 0;

// FPS related variables
GLfloat fps = 0;
int fpsCurrentTime = 0, fpsPreviousTime = 0, fpsFrameCount = 0;

// Terrain textures
lbj_Arraym terrainMats;

// Models
lbj_Model planeModel, nexusModel, carModel, mengerModel;

// Music stream
int musicStream;
int musicIsPlaying = false;
int musicButtonWasPressed = false;

// Box position
vect_Vector boxPos;
// Box game is running
int boxIsEnabled = false;
// Small timer to change direction
int boxTimer = 0;
// Box angle
int boxAngle = 25;

void drawWallsAndFloor(void)
{
	GLdouble i, j;
	// The walls and floor will be a (20 / nr) * (20 / nr) grid
	GLdouble nr = 0.4;

	// Enables textures
	glEnable(GL_TEXTURE_2D);

	// Draws floor
	lbj_LoadMaterial(terrainMats.array[0]);
	glBegin(GL_QUADS);
		for (i = 0; i < 19.99; i += nr) {
			for (j = 0; j < 19.99; j += nr) {
				glNormal3f(0, 1, 0);
				glTexCoord2f(0.5 * i, 0.5 * j);
				glVertex3f(i, 0, j);
				glTexCoord2f(0.5 * i, 0.5 * (j + nr));
				glVertex3f(i, 0, j + nr);
				glTexCoord2f(0.5 * (i + nr), 0.5 * (j + nr));
				glVertex3f(i + nr, 0, j + nr);
				glTexCoord2f(0.5 * (i + nr), 0.5 * j);
				glVertex3f(i + nr, 0, j);
			}
		}
	glEnd();

	// Draws walls
	lbj_LoadMaterial(terrainMats.array[1]);
	glBegin(GL_QUADS);
		for (i = 0; i < 19.99; i += nr) {
			for (j = 0; j < 19.99; j += nr) {
				// Wall 1
				glNormal3f(0, 0, 1);
				glTexCoord2f(0.5 * i, 0.125 * j);
				glVertex3f(i, 0.25 * j, 0);
				glTexCoord2f(0.5 * i, 0.125 * (j + nr));
				glVertex3f(i, 0.25 * (j + nr), 0);
				glTexCoord2f(0.5 * (i + nr), 0.125 * (j + nr));
				glVertex3f(i + nr, 0.25 * (j + nr), 0);
				glTexCoord2f(0.5 * (i + nr), 0.125 * j);
				glVertex3f(i + nr, 0.25 * j, 0);

				// Wall 2
				glNormal3f(0, 0, -1);
				glTexCoord2f(0.5 * i, 0.125 * j);
				glVertex3f(i, 0.25 * j, 20);
				glTexCoord2f(0.5 * i, 0.125 * (j + nr));
				glVertex3f(i, 0.25 * (j + nr), 20);
				glTexCoord2f(0.5 * (i + nr), 0.125 * (j + nr));
				glVertex3f(i + nr, 0.25 * (j + nr), 20);
				glTexCoord2f(0.5 * (i + nr), 0.125 * j);
				glVertex3f(i + nr, 0.25 * j, 20);

				// Wall 3
				glNormal3f(1, 0, 0);
				glTexCoord2f(0.5 * i, 0.125 * j);
				glVertex3f(0, 0.25 * j, i);
				glTexCoord2f(0.5 * i, 0.125 * (j + nr));
				glVertex3f(0, 0.25 * (j + nr), i);
				glTexCoord2f(0.5 * (i + nr), 0.125 * (j + nr));
				glVertex3f(0, 0.25 * (j + nr), i + nr);
				glTexCoord2f(0.5 * (i + nr), 0.125 * j);
				glVertex3f(0, 0.25 * j, i + nr);

				// Wall 4
				glNormal3f(-1, 0, 0);
				glTexCoord2f(0.5 * i, 0.125 * j);
				glVertex3f(20, 0.25 * j, i);
				glTexCoord2f(0.5 * i, 0.125 * (j + nr));
				glVertex3f(20, 0.25 * (j + nr), i);
				glTexCoord2f(0.5 * (i + nr), 0.125 * (j + nr));
				glVertex3f(20, 0.25 * (j + nr), i + nr);
				glTexCoord2f(0.5 * (i + nr), 0.125 * j);
				glVertex3f(20, 0.25 * j, i + nr);
			}
		}
	glEnd();

	// Draws ceiling
	lbj_LoadMaterial(terrainMats.array[1]);
	glBegin(GL_QUADS);
		for (i = 0; i < 19.99; i += nr) {
			for (j = 0; j < 19.99; j += nr) {
				glNormal3f(0, -1, 0);
				glTexCoord2f(i, j);
				glVertex3f(i, 5, j);
				glTexCoord2f(i, j + nr);
				glVertex3f(i, 5, j + nr);
				glTexCoord2f(i + nr, j + nr);
				glVertex3f(i + nr, 5, j + nr);
				glTexCoord2f(i + nr, j);
				glVertex3f(i + nr, 5, j);
			}
		}
	glEnd();

	// Disables textures
	glDisable(GL_TEXTURE_2D);
}

// This will draw a 100 by 100 plane, the camera being always in the middle of it. Gives the impression of an infinite world
void drawGround() {
	int centerX = (int) mot_GetEyePos().x, centerY = (int) mot_GetEyePos().z;
	int i, j;
	glEnable(GL_TEXTURE_2D);
	lbj_LoadMaterial(terrainMats.array[2]);
	glPushMatrix();
		glTranslatef(0, -0.01, 0);
		glBegin(GL_QUADS);
			glNormal3f(0, 1, 0);
			for (i = 0; i < 100; i++) {
				for (j = 0; j < 100; j++) {
					glTexCoord2f(1, 0);
					glVertex3f(centerX + i - 50, 0, centerY + j - 50);
					glTexCoord2f(1, 1);
					glVertex3f(centerX + i - 50 + 1, 0, centerY + j - 50);
					glTexCoord2f(0, 1);
					glVertex3f(centerX + i - 50 + 1, 0, centerY + j - 50 + 1);
					glTexCoord2f(0, 0);
					glVertex3f(centerX + i - 50, 0, centerY + j - 50 + 1);
				}
			}
		glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void display(void)
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	mot_MoveCamera();
	mot_SetCamera();
	// Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	// Loads the default material
	lbj_LoadDefaultMaterial();

	// Reset game if button pressed
	if (!boxIsEnabled && (mot_GetKeyStatus('b') || mot_GetKeyStatus('B')))
	{
		mot_TeleportCamera(5, mot_GetConstant(MOT_EYE_HEIGHT), 5);
		boxIsEnabled = !boxIsEnabled;
	}

	// Draws ground
	drawGround();

	if (!boxIsEnabled) {
		// Draws the room
		drawWallsAndFloor();

		// Draws and animates a cube
		glPushMatrix();
		GLdouble k = (a2 - ((int)a2 / 90) * 90) * 2 * DEG_TO_RAD;
		glTranslatef(5 + 4 * sin(a1 * DEG_TO_RAD), 0.5 + sin(k) * (sqrt(2) / 2 - 0.5), 5 + 4 * cos(a1 * DEG_TO_RAD));
		glRotatef(a2, -sin(a1 * DEG_TO_RAD), 0, -cos(a1 * DEG_TO_RAD));
		glRotatef(a1, 0, 1, 0);
		glutSolidCube(1);
		glPopMatrix();

		// Draws Plane on ground
		glPushMatrix();
		glTranslatef(17, 1, 3);
		glRotatef(a1, 0, 1, 0);
		glScalef(0.4, 0.4, 0.4);
		lbj_DrawModelVBO(planeModel);
		glPopMatrix();

		// Draws Plane that flies
		glPushMatrix();
		glTranslatef(10 + 7 * sin(a1 * DEG_TO_RAD), 4 + 0.5 * sin(a1 * DEG_TO_RAD), 10 + 7 * cos(a1 * DEG_TO_RAD));
		glRotatef(a1 - 90, 0, 1, 0);
		glScalef(0.4, 0.4, 0.4);
		lbj_DrawModelVBO(planeModel);
		glPopMatrix();

		// Draws car
		glPushMatrix();
		glTranslatef(5, 0, 15);
		glRotatef(45, 0, 1, 0);
		glScalef(1.5, 1.5, 1.5);
		lbj_DrawModelVBO(carModel);
		glPopMatrix();

		// Draws Nexus
		glPushMatrix();
		glTranslatef(17, 1.5, 10);
		glRotatef(-90, 0, 1, 0);
		glScalef(0.01, 0.01, 0.01);
		lbj_DrawModelVBO(nexusModel);
		glPopMatrix();

		// Draws Menger Sponge
		glPushMatrix();
		glTranslatef(12, 0, 15);
		glRotatef(45, 0, 1, 0);
		glScalef(0.05, 0.05, 0.05);
		glTranslatef(-27, 0, -27);
		lbj_DrawModelVBO(mengerModel);
		glPopMatrix();
	}
	// Draw and move box away from player
	else {
		// Player position
		vect_Vector player = mot_GetEyePos();
		// Get direction away from player
		vect_Vector dir = vect_Substract(boxPos, player);

		// Check if distance is too small
		if (sqrt(pow(player.x - boxPos.x, 2) + pow(player.z - boxPos.z, 2)) < 5) {
			// The box stays on the ground
			dir.y = 0;
			// After some time rotate the direction randomly to make the chase more difficult
			boxTimer++;
			if (boxTimer >= 150) {
				boxTimer = 0;
				if (rand() % 2) {
					boxAngle = -boxAngle;
				}
			}
			dir = vect_Rotate(dir, boxAngle, 0, 1, 0);
			// Make its speed a little slower that the player's
			vect_Normalize(&dir);
			dir = vect_Multiply(dir, mot_GetConstant(MOT_MAX_SPEED) / 100 * 1.25);
			// Move the box
			boxPos = vect_Add(boxPos, dir);
		}
		else {
			dir = vect_Rotate(dir, boxAngle, 0, 1, 0);
		}

		// If the player catches the box
		if (sqrt(pow(player.x - boxPos.x, 2) + pow(player.z - boxPos.z, 2)) < sqrt(2)) {
			boxIsEnabled = false;
			boxPos = vect_Create(0, 0.5, 0);
			mot_TeleportCamera(5, mot_GetConstant(MOT_EYE_HEIGHT), 5);
		}
		// Draw box
		glEnable(GL_TEXTURE_2D);
		lbj_LoadMaterial(terrainMats.array[2]);
		glPushMatrix();
			glTranslatef(boxPos.x, boxPos.y, boxPos.z);
			glRotatef(atan(dir.x / dir.z) * 180 / PI, 0, 1, 0);
			glutSolidCube(1);
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
	}

	// Swap buffers in GPU
	glutSwapBuffers();
}

void reshape(int width, int height)
{
	// Set the view-port to the full window size
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	// Load the projection matrix
	glMatrixMode(GL_PROJECTION);
	// Clear all the transformations on the projection matrix
	glLoadIdentity();
	// Set the perspective according to the window size
	gluPerspective(70, (GLdouble) width / (GLdouble) height, 0.1, 60000);
	// Load back the model-view matrix
	glMatrixMode(GL_MODELVIEW);
}

void initialize(void)
{
	// Set the background to light gray
	glClearColor(0.8, 0.8, 0.8, 1);
	// Enables depth test
	glEnable(GL_DEPTH_TEST);
	// Disable color material
	glDisable(GL_COLOR_MATERIAL);
	// Enable normalize 
	glEnable(GL_NORMALIZE);
	// Enable blend and transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Select a smooth shading
	glShadeModel(GL_SMOOTH);

	// Sets the light
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	// Enables lighting
	glEnable(GL_LIGHTING);
	// Enables the light
	glEnable(GL_LIGHT0);

	// Enable fog
	//glEnable(GL_FOG);
	// Set fog formula
	glFogi(GL_FOG_MODE, GL_EXP2);
	// Set fog density
	glFogf (GL_FOG_DENSITY, 0.04);
	// Set fog start (only for GL_LINEAR mode)
	glFogf(GL_FOG_START, 10);
	// Set fog end (only for GL_LINEAR mode)
	glFogf(GL_FOG_END, 25);
	// Set fog color
	glFogfv(GL_FOG_COLOR, fogColor);
	// Set fog quality
	glHint(GL_FOG_HINT, GL_NICEST);

	// The parameter should be 1 / AverageFPS
	mot_Init(1 / 80.0);
	// Teleport camera
	mot_TeleportCamera(5, mot_GetConstant(MOT_EYE_HEIGHT), 5);
	//mot_SetState(MOT_IS_OP, true);

	// Initialize box position
	boxPos = vect_Create(0, 0.5, 0);

	// Print stats
	lbj_PrintStats(true);
	// Loads the default material
	lbj_LoadDefaultMaterial();

	// Loads terrain materials
	lbj_SetPaths("./data/models/", "./data/materials/", "./data/textures/");
	lbj_LoadMTLToMaterials("terrain.mtl", &terrainMats, 1);

	// Load models
	lbj_SetPaths("./data/models/", "./data/models/mtl/", "./data/models/tex/");
	lbj_SetFlipping(0, 1, 0, 0, 0);

	// Plane
	lbj_LoadOBJToModel("SimplePlane.obj", &planeModel);
	lbj_CreateVBO(&planeModel, 1);

	// Nexus
	lbj_LoadOBJToModel("Nexus.obj", &nexusModel);
	lbj_CreateVBO(&nexusModel, 1);

	// Car
	lbj_SetFlipping(0, 1, 1, 0, 0);
	lbj_LoadOBJToModel("Avent.obj", &carModel);
	lbj_CreateVBO(&carModel, 0);
	lbj_SetFlipping(0, 1, 0, 0, 0);

	// Menger Sponge
	lbj_LoadOBJToModel("menger_sponge_4.obj", &mengerModel);
	lbj_CreateVBO(&mengerModel, 0);

	// Loads the shaders
	loadShaders("./src/vshader.vsh", GL_VERTEX_SHADER, "./src/fshader.fsh", GL_FRAGMENT_SHADER);
}

void tick(int value)
{
	a1+= 5 * 0.1;
	if (a1 >= 360)
		a1 -= 360;

	a2 += 5 * 0.62831;
	if (a2 >= 360)
		a2 -= 360;
	
	// Pauses or unpauses music
	int isPressed = mot_GetKeyStatus('m') || mot_GetKeyStatus('M');
	if (isPressed && musicButtonWasPressed != isPressed) {
		musicIsPlaying = !musicIsPlaying;
		if (musicIsPlaying) {
			BASS_ChannelPlay(musicStream, FALSE);
		}
		else {
			BASS_ChannelPause(musicStream);
		}
	}
	musicButtonWasPressed = isPressed;

	// Calls the display() function
	glutPostRedisplay();

	// FPS calculus

	// Increase frame count
	fpsFrameCount++;
	// Get the number of milliseconds since glutInit called (or first call to glutGet(GLUT_ELAPSED_TIME))
	fpsCurrentTime = glutGet(GLUT_ELAPSED_TIME);
	// Calculate time passed
	int timeInterval = fpsCurrentTime - fpsPreviousTime;

	if (timeInterval >= 1000)
	{
		// Calculate the number of frames per second
		fps = fpsFrameCount / (float) timeInterval * 1000;
		// Set time
		fpsPreviousTime = fpsCurrentTime;
		// Reset frame count
		fpsFrameCount = 0;
		// Change the window's title
		char title[50];
		if (mot_GetState(MOT_IS_PAUSED)){
			sprintf(title, "Timix Paused | FPS: %f", fps);
		}
		else {
			sprintf(title, "Timix | FPS: %f", fps);
		}
		glutSetWindowTitle(title);
	}

	// Waits 10 ms
	glutTimerFunc(10, tick, 0);
}

// Called when the song ends
void CALLBACK replayMusic(HSYNC handle, DWORD channel, DWORD data, void *user) {
	// Replays the song
	BASS_ChannelPlay(channel, TRUE);
}

// A simple function to load the BASS library and play a song
void bass(void) {
	// Initializes the library
	BASS_Init(-1, 44100, 0, 0, NULL);
	// Creates a stream from a file (the stream is some sort of ID)
	musicStream = BASS_StreamCreateFile(FALSE, "data/sound/Tritonal  Paris Blohm ft Sterling Fox - Colors Culture Code Remix.mp3", 0, 0, 0);
	// Adds an "Event Listener" (it is called sync) to detect when the song ends
	BASS_ChannelSetSync(musicStream, BASS_SYNC_MIXTIME | BASS_SYNC_END, 0, replayMusic, 0);
	// Plays the stream
	if (musicIsPlaying) {
		BASS_ChannelPlay(musicStream, TRUE);
	}
}

int main(int argc, char *argv[])
{
	printf("===================================================\n");
	// Initialize OpenGL, create a window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Timix");
	glewExperimental = GL_TRUE;

	// Initialize GLEW
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		// Problem: glewInit failed, something is seriously wrong.
		ctxt_PrintColored(CTXT_FOREGROUND_LIGHT_RED, "GLEW Error: %s\n", glewGetErrorString(error));
		printf("Press enter to exit...\n");
		getchar();
		exit(0);
	}

	// Check OpenGL version
	int version = 0;
	sscanf((char *)glGetString(GL_VERSION), "%d", &version);
	ctxt_ChangeColor(CTXT_FOREGROUND_LIGHT_AQUA);
	printf("OpenGL version: %s\n\n", glGetString(GL_VERSION));
	ctxt_RestoreColor();
	if (version < 3) {
		ctxt_PrintColored(CTXT_FOREGROUND_LIGHT_RED, "This application needs a minimum OpenGL version of 3.x.\nPlease update graphics card driver.\n\n");
		printf("Press enter to exit...\n");
		getchar();
		exit(0);
	}
	
	initialize();

	bass();

	// Event listeners
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	// Starts main timer
	glutTimerFunc(10, tick, 0);

	glutMainLoop();
	return 0;
}