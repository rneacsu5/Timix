#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "../include/utility.h"
#include "../include/lobjder.h"
#include "../include/motion.h"
#ifdef _WIN32

#include <bass.h>

#else

#include "../include/bass.h"

#endif // _WIN32

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
lbj_Model planeModel, nokiaModel, carModel, nexusModel;

// Music stream
int musicStream;
int musicIsPlaying = true;
int musicButtonWasPressed = false;

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
	// Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	// Loads the default material
	lbj_LoadDefaultMaterial();

	// Draws and animates a cube
	glPushMatrix();
		GLdouble k = (a2 - ((int) a2 / 90) * 90) * 2 * DEG_TO_RAD;
		glTranslatef(5 + 4 * sin(a1 * DEG_TO_RAD), 0.5 + sin(k) * (sqrt(2) / 2 - 0.5), 5 + 4 * cos(a1 * DEG_TO_RAD));
		glRotatef(a2, -sin(a1 * DEG_TO_RAD), 0, -cos(a1 * DEG_TO_RAD));
		glRotatef(a1, 0, 1, 0);
		glutSolidCube(1);
	glPopMatrix();

	// Draws the room
	drawWallsAndFloor();

	// Draws ground
	drawGround();

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
		glRotatef(a1, 0 , 1, 0);
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

	// Draws Nokia
	glPushMatrix();
		glTranslatef(3, 1.5, 3);
		glRotatef(a1, 0, 1, 0);
		glScalef(0.005, 0.005, 0.005);
		glRotatef(90, 1, 0, 0);
		lbj_DrawModelVBO(nokiaModel);
	glPopMatrix();

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
	// Enable Blend and transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Sets the light
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	// Enables lighting
	glEnable(GL_LIGHTING);
	// Enables the light
	glEnable(GL_LIGHT0);
	// Select a smooth shading
	glShadeModel(GL_SMOOTH);
	// Loads the default material
	lbj_LoadDefaultMaterial();

	// Enable fog
	// glEnable(GL_FOG);
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

	// Teleport camera
	mot_TeleportCamera(5, MOT_EYE_HEIGHT, 5);

	// Loads terrain materials
	lbj_SetPaths("./data/models/", "./data/materials/", "./data/textures/");
	lbj_SetFlipping(0, 1, 0, 0, 0);
	lbj_LoadMTLToMaterials("terrain.mtl", &terrainMats, 1);

	// Load models
	lbj_SetPaths("./data/models/", "./data/models/materials/", "./data/models/textures/");

	// Plane
	lbj_LoadOBJToModel("SimplePlane.obj", &planeModel);
	lbj_CreateVBO(&planeModel, 1);

	// Nexus
	lbj_LoadOBJToModel("Nexus.obj", &nexusModel);
	lbj_CreateVBO(&nexusModel, 1);

	// Nokia
	lbj_LoadOBJToModel("sonyericsson-w9600-midres.obj", &nokiaModel);
	lbj_CreateVBO(&nokiaModel, 0);

	// Car
	lbj_SetFlipping(0, 1, 1, 0, 0);
	lbj_LoadOBJToModel("Avent.obj", &carModel);
	lbj_CreateVBO(&carModel, 0);
	lbj_SetFlipping(0, 1, 0, 0, 0);

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
	// Get the number of milliseconds since glutInit called (or first call to glutGet(GLUT ELAPSED TIME))
	fpsCurrentTime = glutGet(GLUT_ELAPSED_TIME);
	// Calculate time passed
	int timeInterval = fpsCurrentTime - fpsPreviousTime;

	if (timeInterval > 1000)
	{
		// Calculate the number of frames per second
		fps = fpsFrameCount / (float) timeInterval * 1000;
		// Set time
		fpsPreviousTime = fpsCurrentTime;
		// Reset frame count
		fpsFrameCount = 0;
		// Change the window's title
		char title[50];
		if (mot_GetIsPaused()){
			sprintf(title, "Epic Game Paused | FPS: %f", fps);
		}
		else {
			sprintf(title, "Epic Game | FPS: %f", fps);
		}
		glutSetWindowTitle(title);
	}

	// Waits 10 ms
	glutTimerFunc(10, tick, value + 1);
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
	BASS_ChannelPlay(musicStream, TRUE);
}

int main(int argc, char *argv[])
{
	printf("===================================================\n");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Epic Game");
	glewExperimental = GL_TRUE;
	glewInit();

	// The parameter should be 1 / AverageFPS
	mot_Init(1 / 90.0);

	// Event listeners
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	
	initialize();

	bass();

	// Starts main timer
	glutTimerFunc(10, tick, 0);

	glutMainLoop();
	return 0;
}