#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include "../include/bitmap.h"
#include "../include/lobjder.h"


// A simple C implementation of std::vector class (This is not mine, I adapted one for this project)

// Initializes the Array and allocates memory
void initArrayv(Arrayv *a, size_t initialSize) {
	a->array = (vector3d *) malloc(initialSize * sizeof(vector3d));
	a->used = 0;
	a->size = initialSize;
}

// Adds an element to the array and reallocates memory if needed
void insertArrayv(Arrayv *a, vector3d element) {
	if (a->used == a->size) {
		a->size *= 2;
		a->array = (vector3d *) realloc(a->array, a->size * sizeof(vector3d));
	}
	a->array[a->used++] = element;
}

// Frees the memory and resets the array
void freeArrayv(Arrayv *a) {
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

// Same as above but for a face array
void initArrayf(Arrayf *a, size_t initialSize) {
	int i;
	for (i = 0; i < 4; i++) {
		a->array[i] = (vector3i *) malloc(initialSize * sizeof(vector3i));
	}
	a->used = 0;
	a->size = initialSize;
}

void insertArrayf(Arrayf *a, vector3i element[4]) {
	int i;
	if (a->used == a->size) {
		a->size *= 2;
		for (i = 0; i < 4; i++) {
			a->array[i] = (vector3i *) realloc(a->array[i], a->size * sizeof(vector3i));
		}
	}
	for (i = 0; i < 4; i++) {
		a->array[i][a->used] = element[i];
	}
	a->used++;
}

void freeArrayf(Arrayf *a) {
	int i;
	for (i = 0; i < 4; i++) {
		free(a->array[i]);
		a->array[i] = NULL;
	}
	a->used = a->size = 0;
}

void initArraym(Arraym *a, size_t initialSize) {
	a->array = (Material *) malloc(initialSize * sizeof(Material));
	a->used = 0;
	a->size = initialSize;
}

void insertArraym(Arraym *a, Material mat) {
	if (a->used == a->size) {
		a->size *= 2;
		a->array = (Material *) realloc(a->array, a->size * sizeof(Material));
	}
	a->array[a->used++] = mat;
}

void freeArraym(Arraym *a) {
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

// Loads a .obj file to a model
void loadOBJToModel(char * filename, Model * model) {
	// Opens file in read-text mode
	FILE * fp = fopen(filename, "rt");
	if (fp == NULL) {
		printf("Failed to open %s. Aborting.\n", filename);
		exit(1);
	}
	// Initializes the model's arrays
	initArrayv(&(*model).v, 10);
	initArrayv(&(*model).vt, 10);
	initArrayv(&(*model).vn, 10);	
	initArrayf(&(*model).f, 10);
	initArraym(&(*model).mats, 10);

	// Now we need to read the file line by line
	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		// The v flag is a vertice
		if (strncmp(line, "v ", 2) == 0) {
			vector3d vect;
			sscanf(line, "%*s %lf %lf %lf", &vect.x, &vect.y, &vect.z);
			insertArrayv(&(*model).v, vect);
		}
		// The vt flag is a texture coordonate
		else if (strncmp(line, "vt", 2) == 0) {
			vector3d vect;
			sscanf(line, "%*s %lf %lf %lf", &vect.x, &vect.y, &vect.z);
			insertArrayv(&(*model).vt, vect);
		}
		// The vn flag is a normal
		else if (strncmp(line, "vn", 2) == 0) {
			vector3d vect;
			sscanf(line, "%*s %lf %lf %lf", &vect.x, &vect.y, &vect.z);
			insertArrayv(&(*model).vn, vect);
		}
		// The f flag is a face
		else if (strncmp(line, "f ", 2) == 0) {
			vector3i vct[4]; // Output vector
			char point[128]; // A vertice/texture/normal sequence

			// Note that there are four formats:

			// <1>     f 145 679 12
			// <2>     f 145/32 679/45 12/64
			// <3>     f 145/32/457 679/45/4799 12/64/146
			// <4>     f 145//457 679//4799 12//146

			// We need to analyze each one

			int i;
			char* p[4]; // Pointers to blank spaces
			p[0] = strchr(line, 32);
			for (i = 0; i < 4 ; i++) {
				if (i == 3) {
					if (p[3] != NULL)
						strcpy(point, p[3] + 1);
					else {
						vct[3] = vct[2];
						break;
					}
				}
				else {
					p[i + 1] = strchr(p[i] + 1, 32);
					if (p[i + 1] != NULL) {
						strncpy(point, p[i] + 1, p[i + 1] - p[i] - 1); // Take a sequence
						point[p[i + 1] - p[i] - 1] = '\0'; // Add the null character
					}
					else 
						strcpy(point, p[i] + 1);
				}

				char * c1; // Pointer to first forward-slash
				char * c2; // Pointer to second forward-slash
				// These two poiters can be NULL so this is how we figure out the format used

				c1 = strchr(p[i] + 1, '/'); // Check for any forward-slash
				if (c1 == NULL) {
					// None found. This is format <1>
					sscanf(point, "%d", &vct[i].x);
					vct[i].y = vct[i].z = 0;
				}
				else {
					// Found the first one. Lets check for the second
					c2 = strchr(c1 + 1, '/');
					if (c2 == NULL) {
						// Second one is missing. Format <2>
						sscanf(point, "%d/%d", &vct[i].x, &vct[i].y);
						vct[i].z = 0;
					}
					else {
						// Found the second one
						// Let's check if they are next to each other
						if (c2 == c1 + 1) {
							// Format <4>
							sscanf(point, "%d//%d", &vct[i].x, &vct[i].z);
							vct[i].y = 0;
						}
						else {
							// Format <3>
							sscanf(point, "%d/%d/%d", &vct[i].x, &vct[i].y, &vct[i].z);
						}
					}
				}
			}
			/*
			printf("%d %d %d %d %d %d %d %d %d %d %d %d\n", 
					vct[0].x, vct[0].y, vct[0].z, vct[1].x, 
					vct[1].y, vct[1].z, vct[2].x, vct[2].y, 
					vct[2].z, vct[3].x, vct[3].y, vct[3].z);
			*/

			insertArrayf(&(*model).f, vct);
		}
		else {
			// Found something else. Maybe a comment?
		}
	}

	// Print Stats
	printf("==========\n");
	printf("Loaded %s.\n", filename);
	printf("Contained: %zd vertice, %zd texture coord, %zd normals, %zd faces\n", 
			model->v.used, 
			model->vt.used, 
			model->vn.used, 
			model->f.used);
	printf("Allocated %zd bytes of memory\n", 
			model->v.size * sizeof(vector3d) + 
			model->vt.size * sizeof(vector3d) + 
			model->vn.size * sizeof(vector3d) + 
			model->f.size * sizeof(vector3i) * 4);

	// Close file
	fclose(fp);
}

// Draws a model
void drawModel(Model * model) {
	int i, j, a, b, c;
	glBegin(GL_QUADS);
		for (i = 0; i < model->f.used; i++) {
			for (j = 0; j < 4; j++) {
				a = model->f.array[j][i].x - 1; // Index of vertice
				b = model->f.array[j][i].y - 1; // Index of texture coordonates
				c = model->f.array[j][i].z - 1; // Index of normal
				// Substact one because the first vertice,normal, etc. starts at 1 in an .obj file

				if (c >= 0) {
					glNormal3f(model->vn.array[c].x, model->vn.array[c].y, model->vn.array[c].z);
				}
				if (b >= 0) {
					glTexCoord2f(model->vt.array[b].x, model->vt.array[b].y);
				}
				if (a >= 0) {
					glVertex3f(model->v.array[a].x, model->v.array[a].y, model->v.array[a].z);
				}
			}
		}
	glEnd();
}

void loadMTLToMaterials(char * filename, Arraym * mat) {
	// Opens file in read-text mode
	FILE * fp = fopen(filename, "rt");
	if (fp == NULL) {
		printf("Failed to open %s. Aborting.\n", filename);
		exit(1);
	}

	// TODO: Finish this function

}

void loadMaterial(Material * mat) {

	// TODO: Create function

}