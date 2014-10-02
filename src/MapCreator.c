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

/* 
	The game will have multiple puzzle levels (or worlds) that comprise only of cubes (1 x 1 x 1 cubes)
	Because our world is made out of cubes, this will simplify the collision tests and creation of levels
	We will use the MapCreator to create this levels and save them in a ".map" file 

	So lets explain the .map file format ant the world format
		The file starts with a header (MAP_FileHeader) that contains basic stuffs about the world
		Then a chain of cubes (MAP_FileCube) follows
		After that a series of special triggers and animations
		Note the "MAP_File..." format
	Then we need to take all the cubes and create a tri-dimensional array (MAP_Data) an put this among others (MAP_Info, MAP_Triggers, etc.) inside a map (MAP_Map)
	For example myMap.Data.array[5][7][2] will represent the cube located at (5, 7, 2). All coordonates must and will be positive.


*/

// A header containing information about the file. It is located at the begining of the .map file
typedef struct
{
	// Number of cubes on the respective axis
	int numberOfCubesX;
	int numberOfCubesY;
	int numberOfCubesZ;
}MAP_FileHeader;

// Represents a single cube loaded from the file
typedef struct
{
	// Coordonates
	int x, y, z;
	// The type of the cube
	int type;
}MAP_FileCube;

// Represents a single cube from the world
typedef struct
{
	// The type of the cube
	int type;
}MAP_Cube;

// Contains all the cubes from the world
typedef struct
{
	MAP_Cube* array[3];
	size_t used[3];
	size_t size[3];
}MAP_Data;

// Some info about the world 
typedef struct
{
	// Number of cubes on each axis
	int numOfCubes[3];
}MAP_Info;

typedef struct
{
	// Info about the world
	MAP_Info Info;
	// All the cubes
	MAP_Data Data;
}MAP_Map;

void initArrayData(MAP_Data* a, size_t initialSize[3])
{
	int i;
	for (i = 0; i < 3; i++) {
		if (initialSize[i] <= 0)
			a->size[i] = 1;
		else 
			a->size[i] = initialSize[i];
		a->array[i] = (MAP_Cube*)malloc(a->size[i] * sizeof(MAP_Cube));
		a->used[i] = 0;
	}
}

void resizeArrayData(MAP_Data* a, size_t size[3])
{
	int i;
	for (i = 0; i < 3; i++) {
		if (size[i] <= 0)
			a->size[i] = 1;
		else
			a->size[i] = size[i];
		a->array[i] = (MAP_Cube*)realloc(a->array[i], a->size[i] * sizeof(MAP_Cube));
		a->used[i] = 0;
	}
}

// Folder where to store and load maps files
char * mapFolder = "./data/maps/";
// The file name with extension
char * fileName;
// The full path to the file
char * filePath;
// File pointer to the file 
FILE * fp;
// Main map file header
MAP_FileHeader mainMapHeader;
MAP_Data mainMapData;


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
	printf("Enter file name: %s", mapFolder);
	// Input entered
	char * fileN = (char*)malloc(32 * sizeof(char));
	if (scanf("%s", fileN) == 1) {
		// Lets extract the extension
		// Pointer to the last dot
		char * p = NULL;
		char * p1 = strchr(fileN, '.');
		while (p1 != NULL) {
			p = p1;
			p1 = strchr(p1 + 1, '.');
		}
		if (p != NULL) {
			if (strstr(p + 1, "map") != NULL && strlen(p + 1) == 3) {
				// File path is folder path + file name
				filePath = (char *)malloc((strlen(mapFolder) + strlen(fileN) + 1) * sizeof(char));
				strcpy(filePath, mapFolder);
				strcat(filePath, fileN);
				// File name
				fileName = (char *)malloc((strlen(fileN) + 1) * sizeof(char));
				strcpy(fileName, fileN);
				// Try to open file in read-binay mode
				fp = fopen(filePath, "rb");
				if (fp != NULL) {
					// File already exists, lets try to read its content
					if (fread(&mainMapHeader, sizeof(MAP_FileHeader), 1, fp) != 1)
						// Failed
						printf("Error loading map from file \"%s\". Invalid or corrupted .map file\n", filePath);
					else
						// Succeeded
						printf("Successfully loaded map from file\n");
					fclose(fp);
				}
				else
					// No file found
					printf("No such vaid file found, creating one: \"%s\"\n", filePath);
				// Open the file in write-binary mode (this also creates the file if it doesn't exist)
				fp = fopen(filePath, "wb");
				if (fp != NULL) {
					// Everything went well
					printf("Done opening map file for writing\n");
					return 0;
				}
				// Something quite bad happened
				printf("Could not open file \"%s\"\n", filePath);
				return 1;
			}
			// Extension found but it's not .map
			printf("Invalid file extension: \"%s\"\n", p);
			return 1;
		}
		// No dot found so there is no extension
		printf("No file extension in given name\n");
	}
	// Input failed?
	return 1;
}

int main(int argc, char *argv[])
{
	printf("===================================================\n");
	while (openMapFile()) {
		// Tries to open a file
	}
	printf("Starting OpenGL...");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Epic Game");
	glewExperimental = GL_TRUE;
	glewInit();
	mot_SetIsOP(true);

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