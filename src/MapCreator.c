#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "../include/utility.h"
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

// FPS related variables
GLfloat fps = 0;
int fpsCurrentTime = 0, fpsPreviousTime = 0, fpsFrameCount = 0;

typedef struct
{
	int i;
	// TODO: Create structure
}MAP;

char * mapFolder = "./data/maps/";
char * fileName;
char * filePath;
FILE * fp;
MAP mainMap;


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

	// Swap buffers in GPU
	glutSwapBuffers();
}

void reshape(int width, int height)
{
	// Set the view-port to the full window size
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	// Load the projection matrix
	glMatrixMode(GL_PROJECTION);
	// Clear all the transformations on the projection matrix
	glLoadIdentity();
	// Set the perspective according to the window size
	gluPerspective(70, (GLdouble)width / (GLdouble)height, 0.1, 60000);
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

	// Enable fog
	// glEnable(GL_FOG);
	// Set fog formula
	glFogi(GL_FOG_MODE, GL_EXP2);
	// Set fog density
	glFogf(GL_FOG_DENSITY, 0.04);
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

	// Loads the shaders
	loadShaders("./src/vshader.vsh", GL_VERTEX_SHADER, "./src/fshader.fsh", GL_FRAGMENT_SHADER);
}

void tick(int value)
{
	// Calls the display() function
	glutPostRedisplay();

	// FPS calculus

	// Increase frame count
	fpsFrameCount++;
	// Get the number of milliseconds since glutInit called (or first call to glutGet(GLUT ELAPSED TIME))
	fpsCurrentTime = glutGet(GLUT_ELAPSED_TIME);
	// Calculate time passed
	int timeInterval = fpsCurrentTime - fpsPreviousTime;

	if (timeInterval > 1000) {
		// Calculate the number of frames per second
		fps = fpsFrameCount / (float)timeInterval * 1000;
		// Set time
		fpsPreviousTime = fpsCurrentTime;
		// Reset frame count
		fpsFrameCount = 0;
		// Change the window's title
		char title[50];
		if (mot_GetIsPaused()) {
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

int openMapFile(void)
{
	// TODO: Documentation and explanation
	printf("Enter file name: %s", mapFolder);
	char * fileN = (char*)malloc(32 * sizeof(char));
	if (scanf("%s", fileN) == 1) {
		char * p = NULL;
		char * p1 = strchr(fileN, '.');
		while (p1 != NULL) {
			p = p1;
			p1 = strchr(p1 + 1, '.');
		}
		if (p != NULL) {
			if (strstr(p + 1, "map") != NULL && strlen(p + 1) == 3) {
				filePath = (char *)malloc((strlen(mapFolder) + strlen(fileN) + 1) * sizeof(char));
				strcpy(filePath, mapFolder);
				strcat(filePath, fileN);
				fileName = (char *)malloc((strlen(fileN) + 1) * sizeof(char));
				strcpy(fileName, fileN);
				fp = fopen(filePath, "rb");
				if (fp != NULL) {
					if (fread(&mainMap, sizeof(MAP), 1, fp) != 1)
						printf("Error loading map from file \"%s\". Invalid or corrupted .map file\n", filePath);
					else
						printf("Successfully loaded map from file\n");
					fclose(fp);
				}
				else
					printf("No such vaid file found, creating one: \"%s\"\n", filePath);
				fp = fopen(filePath, "wb");
				if (fp != NULL) {
					printf("Done opening map file for writing\n");
					return 0;
				}
				printf("Could not open file \"%s\"\n", filePath);
				return 1;
			}
			printf("Invalid file extension: \"%s\"\n", p);
			return 1;
		}
		printf("No file extension in given name\n");
	}
	return 1;
}

int main(int argc, char *argv[])
{
	printf("===================================================\n");
	while (openMapFile()) {
		// Tryes to open a file
	}
	printf("Starting OpenGL...");
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

	// Starts main timer
	glutTimerFunc(10, tick, 0);

	glutMainLoop();
	return 0;
}