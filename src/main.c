#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../include/utility.h"
#include "../include/lobjder.h"
#include "../include/motion.h"
#include "../include/bass.h"

// C does not support boolean
#define true 1
#define false 0

// Ligth parameters
GLfloat lightPos[] = {10, 4.5, 10, 1};
GLfloat lightAmbient[] = {0.5, 0.5, 0.5, 1};
GLfloat lightDiffuse[] = {0.8, 0.8, 0.8, 1};
GLfloat lightSpecular[] = {0.4, 0.4, 0.4, 1};

GLdouble a1 = 0, a2 = 0, r;

// Terrain textures
lbj_Arraym terrainMats;

// Models
lbj_Model planeModel, nokiaModel, cubeModel, carModel, nexusModel, iphoneModel;


void drawWallsAndFloor(void)
{
	GLdouble i, j;
	// The walls and floor will be a (20 / nr) * (20 / nr) grid
	GLdouble nr = 0.4;

	// Enables textures
	glEnable(GL_TEXTURE_2D);
	// Texture environment
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	lbj_LoadMaterial(&terrainMats.array[0]);
	// Resets the color to white
	glColor3f(1.0,1.0,1.0);
	// Draws floor
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

	lbj_LoadMaterial(&terrainMats.array[1]);
	// Draws walls
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

	lbj_LoadMaterial(&terrainMats.array[1]);
	// Draws ceiling
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
	int centerX = (int) motGetEyePos().x, centerY = (int) motGetEyePos().z;
	int i, j;
	glEnable(GL_TEXTURE_2D);
	lbj_LoadMaterial(&terrainMats.array[2]);
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

void drawCube(GLdouble size) 
{
	int i, j;
	GLdouble s = size / 10;
	glPushMatrix();
		glTranslatef(-size / 2, -size / 2, -size / 2);
		glBegin(GL_QUADS);
			for (i = 0; i < 10; i++) {
				for (j = 0; j < 10; j++) {
					glNormal3f(0, -1, 0);
					glVertex3f(s * i, 0 , s * j);
					glVertex3f(s * (i + 1), 0 , s * j);
					glVertex3f(s * (i + 1) , 0 , s * (j + 1));
					glVertex3f(s * i, 0 , s * (j + 1));

					glNormal3f(0, 0, -1);
					glVertex3f(s * i, s * j, 0);
					glVertex3f(s * (i + 1), s * j, 0);
					glVertex3f(s * (i + 1) , s * (j + 1), 0);
					glVertex3f(s * i, s * (j + 1), 0);

					glNormal3f(1, 0, 0);
					glVertex3f(size, s * i, s * j);
					glVertex3f(size, s * i, s * (j + 1));
					glVertex3f(size, s * (i + 1), s * (j + 1));
					glVertex3f(size, s * (i + 1), s * j);

					glNormal3f(0, 0, 1);
					glVertex3f(s * i, s * j, size);
					glVertex3f(s * i, s * (j + 1), size);
					glVertex3f(s * (i + 1), s * (j + 1), size);
					glVertex3f(s * (i + 1), s * j, size);

					glNormal3f(-1, 0, 0);
					glVertex3f(0, s * i, s * j);
					glVertex3f(0, s * i, s * (j + 1));
					glVertex3f(0, s * (i + 1), s * (j + 1));
					glVertex3f(0, s * (i + 1), s * j);

					glNormal3f(0, 1, 0);
					glVertex3f(s * i, size , s * j);
					glVertex3f(s * (i + 1), size , s * j);
					glVertex3f(s * (i + 1) , size , s * (j + 1));
					glVertex3f(s * i, size , s * (j + 1));
				}
			}
		glEnd();
	glPopMatrix();
}

void display(void)
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	motMoveCamera();
	// Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	// Loads the default material
	lbj_LoadDefaultMaterial();

	// Draws and rotates a cyan teapot
	glPushMatrix();
		glTranslatef(3, 0.6, 15);
		glRotatef(a2, 0, 1, 0);
		glColor3f(0, 1, 1);
		glutSolidTeapot(1);
	glPopMatrix();

	// Draws and animates a green cube
	glPushMatrix();
		GLdouble k = (a2 - ((int) a2 / 90) * 90) * 2 * DEG_TO_RAD;
		glTranslatef(5 + 4 * sin(r), 0.5 + sin(k) * (sqrt(2) / 2 - 0.5), 5 + 4 * cos(r));
		glRotatef(a2, -sin(r), 0, -cos(r));
		glRotatef(a1, 0, 1, 0);
		glColor3f(0,0.5,0);
		drawCube(1);
	glPopMatrix();

	// Draws the room
	drawWallsAndFloor();

	// Draws ground
	drawGround();

	// Draws Plane that flies
	glPushMatrix();
		glTranslatef(10 + 7 * sin(r), 4 + 0.5 * sin(r), 10 + 7 * cos(r));
		glRotatef(a1 - 90, 0, 1, 0);
		glScalef(0.4, 0.4, 0.4);
		lbj_DrawModel(&planeModel);
	glPopMatrix();

	// Draws Plane on ground
	glPushMatrix();
		glTranslatef(17, 1, 3);
		glRotatef(a1, 0, 1, 0);
		glScalef(0.4, 0.4, 0.4);
		lbj_DrawModel(&planeModel);
	glPopMatrix();

	// Draws car
	glPushMatrix();
		glTranslatef(3, 0, 7);
		glRotatef(a1, 0 , 1, 0);
		glRotatef(-90, 1, 0, 0);
		glScalef(0.03, 0.03, 0.03);
		lbj_DrawModel(&carModel);
	glPopMatrix();

	// Draws Nexus
	glPushMatrix();
		glTranslatef(17, 1.5, 10);
		glRotatef(-90, 0, 1, 0);
		glScalef(0.01, 0.01, 0.01);
		lbj_DrawModel(&nexusModel);
	glPopMatrix();

	// Draws Nokia
	glPushMatrix();
		glTranslatef(3, 1.5, 3);
		glRotatef(a1, 0, 1, 0);
		glScalef(0.005, 0.005, 0.005);
		glRotatef(90, 1, 0, 0);
		lbj_DrawModel(&nokiaModel);
	glPopMatrix();

	// Draws cube
	glPushMatrix();
		glTranslatef(7, 0.5, 3);
		glRotatef(a1, 0, 1, 0);
		glScalef(0.5, 0.5, 0.5);
		lbj_DrawModel(&cubeModel);
	glPopMatrix();

	// Draws IPhone
	glPushMatrix();
		glTranslatef(5, 1.5, 3);
		glScalef(0.1, 0.1, 0.1);
		lbj_DrawModel(&iphoneModel);
	glPopMatrix();

	// Swap buffers in GPU
	glutSwapBuffers();
}

void reshape(int width, int height)
{
	// Set the viewport to the full window size
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	// Load the projection matrix
	glMatrixMode(GL_PROJECTION);
	// Clear all the transformations on the projection matrix
	glLoadIdentity();
	// Set the perspective according to the window size
	gluPerspective(70, (GLdouble) width / (GLdouble) height, 0.1, 60000);
	// Load back the modelview matrix
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
	// Loads the default material
	lbj_LoadDefaultMaterial();

	glShadeModel(GL_SMOOTH);

	// Loads terrain materials
	lbj_SetPaths("./data/models/", "./data/materials/", "./data/textures/");
	lbj_SetFlipping(0, 1, 0, 0, 0);

	lbj_LoadMTLToMaterials("terrain.mtl", &terrainMats, 1);

	// Load models
	lbj_SetPaths("./data/models/", "./data/models/materials/", "./data/models/textures/");

	// Plane
	lbj_LoadOBJToModel("SimplePlane.obj", &planeModel);

	// Cube
	lbj_LoadOBJToModel("cube2.obj", &cubeModel);

	// Nexus
	lbj_LoadOBJToModel("Nexus.obj", &nexusModel);

	// Nokia
	lbj_LoadOBJToModel("sonyericsson-w9600-midres.obj", &nokiaModel);

	// Car
	// lbj_LoadOBJToModel("alfa147.obj", &carModel);

	// IPhone 4S
	lbj_LoadOBJToModel("iphone_4s_home_screen.obj", &iphoneModel);

	// Loads the shaders
	loadShaders("./src/vshader.vsh", GL_VERTEX_SHADER, "./src/fshader.fsh", GL_FRAGMENT_SHADER);
}

void tick(int value)
{
	a1+= 5 * 0.1;
	if (a1 >= 360)
		a1 -= 360;
	r = a1 * DEG_TO_RAD;

	a2 += 5 * 0.62831;
	if (a2 >= 360)
		a2 -= 360;
	
	// Calls the display() function
	glutPostRedisplay();

	// Waits 10 ms
	glutTimerFunc(10, tick, value + 1);
}

// Called when the song ends
void replayMusic(HSYNC handle, DWORD channel, DWORD data, void *user) {
	// Replays the song
	BASS_ChannelPlay(channel, TRUE);
}

void bass(void) {
	// Initializes the library
	BASS_Init(-1, 44100, 0, 0, NULL);
	// Creates a stream from a file (the stream is some sort of ID)
	int stream = BASS_StreamCreateFile(FALSE, "data/sound/Tritonal  Paris Blohm ft Sterling Fox - Colors Culture Code Remix.mp3", 0, 0, 0);
	// Adds an "Event Listener" (it is called sync) to detect when the song ends
	BASS_ChannelSetSync(stream, BASS_SYNC_MIXTIME | BASS_SYNC_END, 0, replayMusic, 0);
	// Plays the stream
	BASS_ChannelPlay(stream, TRUE);
}


int main(int argc, char *argv[])
{
	printf("===================================================\n");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Epic Game");
	glewInit();
	motionInit();

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
