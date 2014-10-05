#include <stdio.h>

// A header containing information about the file. It is located at the begining of the .map file
typedef struct
{
	// The name of the map/level
	char name[64];
	// Number of cubes
	unsigned numOfCubes;
} MAP_FileHeader;

// Represents a single cube loaded from the file
typedef struct
{
	// Coordonates
	int x, y, z;
	// The type of the cube
	int type;
} MAP_FileCube;

// Represents a single cube from the world
typedef struct
{
	// The type of the cube
	int type;
} MAP_Cube;

// Contains all the cubes from the world
typedef struct
{
	MAP_Cube* array;
	size_t used[3];
	size_t size[3];
	int offset[3];
} MAP_Data;

// Some info about the world 
typedef struct
{
	// Name of the map/level
	char * name;
	// Number of cubes on each axis
	int numOfCubes[3];
} MAP_Info;

typedef struct
{
	// Info about the world
	MAP_Info Info;
	// All the cubes
	MAP_Data Data;
} MAP_Map;