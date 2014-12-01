#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#include <GL/glew.h>
#include <GL/glut.h>

#include "../include/utility.h"
#define VECTOR_IMPLEMENTATION
#include "../include/vector.h"
#define MOTION_IMPLEMENTATION
#include "../include/motion.h"
#include "../include/MapCreator.h"

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
	Then we need to take all the cubes and create a tri-dimensional array (MAP_Data) and put this among others (MAP_Info, MAP_Triggers, etc.) inside a map (MAP_Map)
	For example getCubeAt(myMap.Data, 5, 7, 2) will represent the cube located at (5, 7, 2). All coordonates must and will be positive.

*/

#define CUBE_AIR 0
#define CUBE_FIXED_SOLID 1

int saveBtnWasPressed = 0;

vect_Vector lastPos;

// Initiate the data array
void initArrayData(MAP_Data* a, unsigned int size[3])
{
	int i;
	for (i = 0; i < 3; i++) {
		if (size[i] <= 0)
			a->size[i] = 1;
		else 
			a->size[i] = size[i];
		a->offset[i] = 0;
	}
	a->array = (MAP_Cube*)malloc((a->size[0] * a->size[1] * a->size[2]) * sizeof(MAP_Cube));
}

// Resizes the data array
void resizeArrayData(MAP_Data* a, unsigned int size[3])
{
	int i;
	for (i = 0; i < 3; i++) {
		if (size[i] <= 0)
			a->size[i] = 1;
		else
			a->size[i] = size[i];
	}
	a->array = (MAP_Cube*)realloc(a->array, (a->size[0] * a->size[1] * a->size[2]) * sizeof(MAP_Cube));
}

// Returns the cube at the given position
MAP_Cube getCubeAt(MAP_Data d, int x, int y, int z)
{
	return d.array[(z + d.offset[2]) + (y + d.offset[1]) * d.size[2] + (x + d.offset[0]) * d.size[2] * d.size[1]];
}

// Sets the cube at the given position
void setCubeAt(MAP_Data *d, int x, int y, int z, MAP_Cube c)
{
	d->array[(z + d->offset[2]) + (y + d->offset[1]) * d->size[2] + (x + d->offset[0]) * d->size[2] * d->size[1]] = c;
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
MAP_Map Map;

// A simple way to draw a map
void drawMap(MAP_Map map)
{
	int a, b, c;
	// "For frenzy"
	for (a = -map.Data.offset[0]; a < (int)map.Data.size[0] - map.Data.offset[0]; a++) {
		for (b = -map.Data.offset[1]; b < (int)map.Data.size[1] - map.Data.offset[1]; b++) {
			for (c = -map.Data.offset[2]; c < (int)map.Data.size[2] - map.Data.offset[2]; c++) {
				if (getCubeAt(map.Data, a, b, c).type != CUBE_AIR) {
					glPushMatrix();
					glTranslatef(a + 0.5, b + 0.5, c + 0.5);
					glutSolidCube(1);
					glPopMatrix();
				}
			}
		}
	}
}

// Initialize an empty map
void initMap(MAP_Map *m, unsigned int size[3])
{
	unsigned long int i;

	// Let's set the name of the map
	m->Info.name = (char*)malloc(64 * sizeof(char));
	strcpy(m->Info.name, "Nexus Rulz");

	// Now for the Data array
	initArrayData(&m->Data, size);

	// The default cube used
	MAP_Cube defaultCube;
	defaultCube.type = CUBE_AIR;

	// Init array with default cube
	for (i = 0; i < size[0] * size[1] * size[2]; i++) {
		m->Data.array[i] = defaultCube;
	}
}

// Loads map from file.
// If anything fails the function returns 1, else 0 is returned
int loadMapFromFile(FILE * fp, MAP_Map * map)
{
	// If file is NULL
	if (fp == NULL) {
		return 1;
	}

	// Read the header
	MAP_FileHeader myHeader;
	if (fread(&myHeader, sizeof(MAP_FileHeader), 1, fp) != 1) {
		return 1;
	}

	// We don't want to directly modify the map given unless everything goes well
	MAP_Map myMap;

	// The cubes from the file
	MAP_FileCube * myCubes = (MAP_FileCube*)malloc(myHeader.numOfCubes * sizeof(MAP_FileCube));
	// Read them
	if (fread(myCubes, sizeof(MAP_FileCube), myHeader.numOfCubes, fp) != myHeader.numOfCubes) {
		return 1;
	}

	// Calculate max and min for each axis
	unsigned int i;
	int min[3] = {0, 0, 0}, max[3] = {0, 0, 0};
	if (myHeader.numOfCubes != 0) {
		min[0] = max[0] = myCubes[0].x;
		min[1] = max[1] = myCubes[0].y;
		min[2] = max[2] = myCubes[0].z;
	}
	for (i = 0; i < myHeader.numOfCubes; i++) {
		if (myCubes[i].x < min[0]) min[0] = myCubes[i].x;
		if (myCubes[i].y < min[1]) min[1] = myCubes[i].y;
		if (myCubes[i].z < min[2]) min[2] = myCubes[i].z;
		if (myCubes[i].x > max[0]) max[0] = myCubes[i].x;
		if (myCubes[i].y > max[1]) max[1] = myCubes[i].y;
		if (myCubes[i].z > max[2]) max[2] = myCubes[i].z;
	}

	// Size is max - min + 1
	unsigned int size[3];
	for (int i = 0; i < 3; i++) {
		size[i] = max[i] - min[i] + 1;
	}

	// Now initialize the map
	initMap(&myMap, size);

	// Offset is -min
	for (int i = 0; i < 3; i++) {
		myMap.Data.offset[i] = -min[i];
	}

	// Get the name of the map
	free(myMap.Info.name);
	myMap.Info.name = (char *)malloc((strlen(myHeader.name) + 1) * sizeof(char));
	strcpy(myMap.Info.name, myHeader.name);

	// Add cubes
	MAP_Cube cube;
	for (i = 0; i < myHeader.numOfCubes; i++) {
		cube.type = myCubes[i].type;
		setCubeAt(&myMap.Data, myCubes[i].x, myCubes[i].y, myCubes[i].z, cube);
	}

	// Everything went well so
	*map = myMap;
	return 0;
}

// Save the map to the file
// If writting fails the function returns 1, else 0 is returned
int saveMapToFile(FILE * fp, MAP_Map map)
{
	// File header
	MAP_FileHeader myH;
	// A file cube
	MAP_FileCube cube;
	// Copy the name
	strcpy(myH.name, map.Info.name);
	// Number of non-air cubes
	myH.numOfCubes = 0;
	// Size of allocated memory for myCubes
	unsigned int used = 10;
	// Array of cubes that will be written to the file
	MAP_FileCube * myCubes = (MAP_FileCube *)malloc(used * sizeof(MAP_FileCube));

	int a, b, c;
	// "For frenzy"
	for (a = -map.Data.offset[0]; a < (int)map.Data.size[0] - map.Data.offset[0]; a++) {
		for (b = -map.Data.offset[1]; b < (int)map.Data.size[1] - map.Data.offset[1]; b++) {
			for (c = -map.Data.offset[2]; c < (int)map.Data.size[2] - map.Data.offset[2]; c++) {
				// If the cube is not air add it to the array
				if (getCubeAt(map.Data, a, b, c).type != CUBE_AIR) {
					// Increment
					myH.numOfCubes++;
					// Realloc memory if needed
					if (myH.numOfCubes > used) {
						used *= 2;
						myCubes = realloc(myCubes, used * sizeof(MAP_FileCube));
					}
					// Add the cube to the array
					cube.x = a; cube.y = b; cube.z = c;
					cube.type = getCubeAt(map.Data, a, b, c).type;
					myCubes[myH.numOfCubes - 1] = cube;
				}
			}
		}
	}

	// Move cursor to start of the file
	fseek(fp, 0, SEEK_SET);
	// Reopen file (clear content)
	fp = freopen(filePath, "wb", fp);
	if (fp == NULL) return 1;
	// Write the header
	if (fwrite(&myH, sizeof(MAP_FileHeader), 1, fp) != 1) return 1;
	// Write the cubes
	if (fwrite(myCubes, sizeof(MAP_FileCube), myH.numOfCubes, fp) != myH.numOfCubes) return 1;
	// Move cursor to start of the file
	fseek(fp, 0, SEEK_SET);

	return 0;
}

// Prompts for a file to open the map
// If anything fails the function returns 1, else 0 is returned
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
				int exists = 0;
				if (fp != NULL) {
					// File already exists, lets try to read its content
					if (loadMapFromFile(fp, &Map)) {
						// Failed
						printf("Error loading map from file \"%s\". Invalid or corrupted .map file\n", filePath);
					}
					else {
						// Succeeded
						exists = 1;
						printf("Successfully loaded map from file\n");
					}
				}
				else {
					// No file found
					printf("No such vaid file found, creating one: \"%s\"\n", filePath);
				}

				// Open the file in write-binary mode (this also creates the file if it doesn't exist)
				if (exists) {
					fp = freopen(filePath, "wb", fp);
				}
				else {
					fp = fopen(filePath, "wb");
					// Init our Map
					int size[3] = {50, 50, 50};
					initMap(&Map, size);
				}

				if (fp != NULL) {
					// Save the map in case the user forgets to save it
					saveMapToFile(fp, Map);
					// Everything went well
					printf("Done.\n");
					return 0;
				}

				// Something bad happened
				printf("Could not open file \"%s\"\n", filePath);
				return 1;
			}
			// Extension found but it's not .map
			printf("Invalid file extension: \"%s\"\n", p);
			return 1;
		}
		// No dot found so there is no extension
		printf("No file extension in given name\n");
		return 1;
	}
	// Input failed?
	return 1;
}

// Get the position of the cube in front of the camera
vect_Vector getSelectedCube(void)
{
	// Get position of eye and target
	vect_Vector pos = mot_GetTargetPos();
	vect_Vector eye = mot_GetEyePos();

	// Move the target sqrt(3) units in front of the camera
	pos = vect_Substract(pos, eye);
	vect_Normalize(&pos);
	pos = vect_Multiply(pos, sqrt(3));
	pos = vect_Add(pos, eye);

	// Transform to int
	pos.x = (int)pos.x;
	pos.y = (int)pos.y;
	pos.z = (int)pos.z;
	return pos;
}

// Changes the block at the eye level
void changeBlock(void)
{
	if (!mot_GetState(MOT_IS_OP)) return;
	int i;
	int resize = 0;

	// Get position
	vect_Vector pos = getSelectedCube();

	// Create a blank map data. It will be used if we need to resize the map
	MAP_Data newData;
	for (i = 0; i < 3; i++) {
		newData.size[i] = Map.Data.size[i];
		newData.offset[i] = Map.Data.offset[i];
	}

	// Check if the block is outside the boundary and resize newMap's size and offset
	if (pos.x < -Map.Data.offset[0]) {
		newData.size[0] += -Map.Data.offset[0] - pos.x;
		newData.offset[0] = -pos.x;
		resize = 1;
	}
	if (pos.y < -Map.Data.offset[1]) {
		newData.size[1] += -Map.Data.offset[1] - pos.y;
		newData.offset[1] = -pos.y;
		resize = 1;
	}
	if (pos.z < -Map.Data.offset[2]) {
		newData.size[2] += -Map.Data.offset[2] - pos.z;
		newData.offset[2] = -pos.z;
		resize = 1;
	}
	if (pos.x > Map.Data.size[0] - Map.Data.offset[0] - 1) {
		newData.size[0] += pos.x - Map.Data.size[0] + Map.Data.offset[0] + 1;
		resize = 1;
	}
	if (pos.y > Map.Data.size[1] - Map.Data.offset[1] - 1) {
		newData.size[1] += pos.y - Map.Data.size[1] + Map.Data.offset[1] + 1;
		resize = 1;
	}
	if (pos.z > Map.Data.size[2] - Map.Data.offset[2] - 1) {
		newData.size[2] += pos.z - Map.Data.size[2] + Map.Data.offset[2] + 1;
		resize = 1;
	}

	// Note that we resize the map only if needed
	if (resize) {
		// Because initArrayData() resets the offset to 0 we need to store it temporarly
		int tempOffset[3];
		for (i = 0; i < 3; i++) {
			tempOffset[i] = newData.offset[i];
		}

		// The map data is initialized with the new size
		initArrayData(&newData, newData.size);

		// Restore offset
		for (i = 0; i < 3; i++) {
			newData.offset[i] = tempOffset[i];
		}

		// The default cube used
		MAP_Cube defaultCube;
		defaultCube.type = CUBE_AIR;

		// Init array with default cube
		unsigned int j;
		for (j = 0; j < newData.size[0] * newData.size[1] * newData.size[2]; j++) {
			newData.array[j] = defaultCube;
		}

		// The content of the original map is copied to the new map
		int a, b, c;
		// "For frenzy"
		for (a = -Map.Data.offset[0]; a < (int)Map.Data.size[0] - Map.Data.offset[0]; a++) {
			for (b = -Map.Data.offset[1]; b < (int)Map.Data.size[1] - Map.Data.offset[1]; b++) {
				for (c = -Map.Data.offset[2]; c < (int)Map.Data.size[2] - Map.Data.offset[2]; c++) {
					setCubeAt(&newData, a, b, c, getCubeAt(Map.Data, a, b, c));
				}
			}
		}

		// Free old data
		free(Map.Data.array);
		// Update new data
		Map.Data = newData;
	}

	// For now we only toggle cubes form air to solid
	MAP_Cube cube;
	if (getCubeAt(Map.Data, pos.x, pos.y, pos.z).type == CUBE_AIR) {
		cube.type = CUBE_FIXED_SOLID;
	}
	else {
		cube.type = CUBE_AIR;
	}
	setCubeAt(&Map.Data, pos.x, pos.y, pos.z, cube);
}

// Draws map boundary and selected cube
void drawGuides(void)
{
	if (!mot_GetState(MOT_IS_OP)) return;
	// Draw Map bound
	glBegin(GL_LINE_LOOP);
		glVertex3f(-Map.Data.offset[0], -Map.Data.offset[1] + Map.Data.size[1], -Map.Data.offset[2]);
		glVertex3f(-Map.Data.offset[0] + Map.Data.size[0], -Map.Data.offset[1] + Map.Data.size[1], -Map.Data.offset[2]);
		glVertex3f(-Map.Data.offset[0] + Map.Data.size[0], -Map.Data.offset[1] + Map.Data.size[1], -Map.Data.offset[2] + Map.Data.size[2]);
		glVertex3f(-Map.Data.offset[0], -Map.Data.offset[1] + Map.Data.size[1], -Map.Data.offset[2] + Map.Data.size[2]);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex3f(-Map.Data.offset[0], -Map.Data.offset[1], -Map.Data.offset[2]);
		glVertex3f(-Map.Data.offset[0] + Map.Data.size[0], -Map.Data.offset[1], -Map.Data.offset[2]);
		glVertex3f(-Map.Data.offset[0] + Map.Data.size[0], -Map.Data.offset[1], -Map.Data.offset[2] + Map.Data.size[2]);
		glVertex3f(-Map.Data.offset[0], -Map.Data.offset[1], -Map.Data.offset[2] + Map.Data.size[2]);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(-Map.Data.offset[0], -Map.Data.offset[1], -Map.Data.offset[2]);
		glVertex3f(-Map.Data.offset[0], -Map.Data.offset[1] + Map.Data.size[1], -Map.Data.offset[2]);

		glVertex3f(-Map.Data.offset[0] + Map.Data.size[0], -Map.Data.offset[1], -Map.Data.offset[2]);
		glVertex3f(-Map.Data.offset[0] + Map.Data.size[0], -Map.Data.offset[1] + Map.Data.size[1], -Map.Data.offset[2]);

		glVertex3f(-Map.Data.offset[0], -Map.Data.offset[1], -Map.Data.offset[2] + Map.Data.size[2]);
		glVertex3f(-Map.Data.offset[0], -Map.Data.offset[1] + Map.Data.size[1], -Map.Data.offset[2] + Map.Data.size[2]);

		glVertex3f(-Map.Data.offset[0] + Map.Data.size[0], -Map.Data.offset[1], -Map.Data.offset[2] + Map.Data.size[2]);
		glVertex3f(-Map.Data.offset[0] + Map.Data.size[0], -Map.Data.offset[1] + Map.Data.size[1], -Map.Data.offset[2] + Map.Data.size[2]);
	glEnd();
	
	// Select cube
	vect_Vector pos = getSelectedCube();
	glPushMatrix();
		glTranslatef(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5);
		glutWireCube(1);
	glPopMatrix();

}

// Handles mouse click events
void mouseClick(int button, int state, int x, int y)
{
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		// Right click
		changeBlock();
	}
}

void doMagic(void)
{
	return; // REMOVE THIS AFTER YOU ARE DONE

	if (mot_GetState(MOT_IS_OP)) {
		return;
	}

	// Player position
	vect_Vector pos = vect_Create(mot_GetEyePos().x - 0.5, mot_GetEyePos().y - mot_GetConstant(MOT_EYE_HEIGHT), mot_GetEyePos().z - 0.5);
	// Integer position
	int x = (int)pos.x, y = (int)pos.y, z = (int)pos.z;
	// Cube to check with
	int a,b,c;
	// Velocity
	vect_Vector velocity = mot_GetVelocity();

	// Take each level (the player is 2 cubes tall)
	for (b = y; b < y + 3; b++) {
		a = x; c = z;
		while (1) {
			// Check for air or if outside bound
			int skip = 0;
			if (a > (int)Map.Data.size[0] + Map.Data.offset[0] ||
				b > (int)Map.Data.size[1] + Map.Data.offset[1] ||
				c > (int)Map.Data.size[2] + Map.Data.offset[2] ||
				a < Map.Data.offset[0] ||
				b < Map.Data.offset[1] ||
				c < Map.Data.offset[2])

				skip = 1;

			if (!skip)
				if (getCubeAt(Map.Data, a, b, c).type == CUBE_AIR) 
					skip = 1;

			if (!skip) {
				// Algoritm that checks for collision and moves player accordingly
				if (pos.x - 1 < a && pos.x > a && velocity.x < 0 && lastPos.y - 1 < b && lastPos.y + 1 > b && lastPos.z - 1 < c && lastPos.z + 1 > c) {
					pos.x = a + 1;
					velocity.x = 0;
				}
				if (pos.x + 1 > a && pos.x < a && velocity.x > 0 && lastPos.y - 1 < b && lastPos.y + 1 > b && lastPos.z - 1 < c && lastPos.z + 1 > c) {
					pos.x = a - 1;
					velocity.x = 0;
				}

				if (pos.y - 1 < b && pos.y > b && velocity.y < 0 && lastPos.x - 1 < a && lastPos.x + 1 > a && lastPos.z - 1 < c && lastPos.z + 1 > c) {
					pos.y = b + 1;
					velocity.y = 0;
				}
				if (pos.y + 1 > b && pos.y < b && velocity.y > 0 && lastPos.x - 1 < a && lastPos.x + 1 > a && lastPos.z - 1 < c && lastPos.z + 1 > c) {
					pos.y = b - 1;
					velocity.y = 0;
				}

				if (pos.z - 1 < c && pos.z > c && velocity.z < 0 && lastPos.x - 1 < a && lastPos.x + 1 > a && lastPos.y - 1 < b && lastPos.y + 1 > b) {
					pos.z = c + 1;
					velocity.z = 0;
				}
				if (pos.z + 1 > c && pos.z < c && velocity.z > 0 && lastPos.x - 1 < a && lastPos.x + 1 > a && lastPos.y - 1 < b && lastPos.y + 1 > b) {
					pos.z = c - 1;
					velocity.z = 0;
				}
			}
			glPushMatrix();
				glTranslatef(a + 0.5, b + 0.5, c + 0.5);
				glutWireCube(1);
			glPopMatrix();
			if (c == z) c++;
			else if (c == z + 1) {
				c--;
				a++;
			}
			if (c == z && a == x + 2) break;
		}
	}
	mot_SetVelocity(velocity);
	mot_TeleportCamera(pos.x + 0.5, pos.y + mot_GetConstant(MOT_EYE_HEIGHT), pos.z + 0.5);
	lastPos = pos;
}

void exitFunc(int exitCode)
{
	exit(exitCode);
}

void display(void)
{
	// Clear the color buffer, restore the background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load the identity matrix, clear all the previous transformations
	glLoadIdentity();
	// Set up the camera
	mot_MoveCamera();
	// Collisions with blocks
	doMagic();
	mot_SetCamera();
	// Set light position
	vect_Vector eyePos = mot_GetEyePos(), targetPos = mot_GetTargetPos(), pos;
	pos = vect_Substract(targetPos, eyePos);
	vect_Normalize(&pos);
	pos = vect_Multiply(pos, -1);
	pos = vect_Add(pos, eyePos);
	lightPos[0] = pos.x;
	lightPos[1] = pos.y;
	lightPos[2] = pos.z;
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	drawMap(Map);

	drawGuides();

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
	glFogf(GL_FOG_DENSITY, 0.04);
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
	mot_SetState(MOT_IS_OP, true);
	mot_SetConstant(MOT_MAX_SPEED, 10);
	mot_ExitFunc(exitFunc);

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
		if (mot_GetState(MOT_IS_PAUSED)) {
			sprintf(title, "Epic Game Paused | FPS: %f", fps);
		}
		else {
			sprintf(title, "Epic Game | FPS: %f", fps);
		}
		glutSetWindowTitle(title);
	}

#ifdef _WIN32
	
	// Save the map if Ctrl-S is pressed
	if (GetKeyState(VK_LCONTROL) < 0 && GetKeyState('S') < 0) {
		if (!saveBtnWasPressed) {
			saveBtnWasPressed = 1;
			if (saveMapToFile(fp, Map)) {
				printf("Error while saving file :(\n");
			}
			else {
				printf("Saved map to file.\n");
			}
		}
	}
	else {
		saveBtnWasPressed = 0;
	}

	// Toggle OP mode
	if (GetKeyState('C') < 0) {
		mot_SetState(MOT_IS_OP, 1);
	}
	if (GetKeyState('R') < 0) {
		mot_SetState(MOT_IS_OP, 0);
	}

#endif // _WIN32

	// Waits 10 ms
	glutTimerFunc(10, tick, value + 1);
}

int main(int argc, char *argv[])
{
	printf("===================================================\n");
	while (openMapFile()); // Tries to open a file
	printf("Starting OpenGL...\n");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Epic Game");
	glewExperimental = GL_TRUE;
	glewInit();

	initialize();

	// Event listeners
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseClick);

	// Starts main timer
	glutTimerFunc(10, tick, 0);

	glutMainLoop();
	return 0;
}