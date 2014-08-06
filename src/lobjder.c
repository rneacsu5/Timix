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
	a->array1 = (vector3i *) malloc(initialSize * sizeof(vector3i));
	a->array2 = (vector3i *) malloc(initialSize * sizeof(vector3i));
	a->array3 = (vector3i *) malloc(initialSize * sizeof(vector3i));
	a->used = 0;
	a->size = initialSize;
}

void insertArrayf(Arrayf *a, vector3i element[3]) {
	if (a->used == a->size) {
		a->size *= 2;
		a->array1 = (vector3i *) realloc(a->array1, a->size * sizeof(vector3i));
		a->array2 = (vector3i *) realloc(a->array2, a->size * sizeof(vector3i));
		a->array3 = (vector3i *) realloc(a->array3, a->size * sizeof(vector3i));
	}
	a->array1[a->used] = element[0];
	a->array2[a->used] = element[1];
	a->array3[a->used] = element[2];
	a->used++;
}

void freeArrayf(Arrayf *a) {
	free(a->array1);
	a->array1 = NULL;
	free(a->array2);
	a->array2 = NULL;
	free(a->array3);
	a->array3 = NULL;
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
			vector3i vct[3]; // Output vector
			char point[128]; // A vertice/texture/normal sequence

			// Note that there are four formats:

			// <1>     f 145 679 12
			// <2>     f 145/32 679/45 12/64
			// <3>     f 145/32/457 679/45/4799 12/64/146
			// <4>     f 145//457 679//4799 12//146

			// We need to analyze each one
			char * p1 = strchr(line, 32); // Pointer to first blank space
			char * p2 = strchr(p1 + 1, 32); // Pointer to second blank space
			char * p3 = strchr(p2 + 1, 32); // Pointer to third blank space
			char * c1; // Pointer to first forward-slash
			char * c2; // Pointer to second forward-slash
			// The last two poiters can be NULL so this is how we figure out the format used

			strncpy(point, p1 + 1, p2 - p1 - 1); // Take the fist sequence
			point[p2 - p1 - 1] = '\0'; // Add the null character
			c1 = strchr(p1 + 1, '/'); // Check for any forward-slash
			if (c1 == NULL) {
				// None found. This is format <1>
				sscanf(point, "%d", &vct[0].x);
				vct[0].y = vct[0].z = 0;
			}
			else {
				// Found the first one. Lets check for the second
				c2 = strchr(c1 + 1, '/');
				if (c2 == NULL) {
					// Second one is missing. Format <2>
					sscanf(point, "%d/%d", &vct[0].x, &vct[0].y);
					vct[0].z = 0;
				}
				else {
					// Found the second one
					// Let's check if they are next to each other
					if (c2 == c1 + 1) {
						// Format <4>
						sscanf(point, "%d//%d", &vct[0].x, &vct[0].z);
						vct[0].y = 0;
					}
					else {
						// Format <3>
						sscanf(point, "%d/%d/%d", &vct[0].x, &vct[0].y, &vct[0].z);
					}

				}
			}

			// Same for the other two points
			strncpy(point, p2 + 1, p3 - p2 - 1);
			point[p3 - p2 - 1] = '\0';
			c1 = strchr(p2 + 1, '/');
			if (c1 == NULL) {
				sscanf(point, "%d", &vct[0].x);
				vct[1].y = vct[1].z = 0;
			}
			else {
				c2 = strchr(c1 + 1, '/');
				if (c2 == NULL) {
					sscanf(point, "%d/%d", &vct[1].x, &vct[1].y);
					vct[1].z = 0;
				}
				else {
					if (c2 == c1 + 1) {
						sscanf(point, "%d//%d", &vct[1].x, &vct[1].z);
						vct[1].y = 0;
					}
					else {
						sscanf(point, "%d/%d/%d", &vct[1].x, &vct[1].y, &vct[1].z);
					}

				}
			}

			strcpy(point, p3 + 1);
			c1 = strchr(p3 + 1, '/');
			if (c1 == NULL) {
				sscanf(point, "%d", &vct[2].x);
				vct[2].y = vct[2].z = 0;
			}
			else {
				c2 = strchr(c1 + 1, '/');
				if (c2 == NULL) {
					sscanf(point, "%d/%d", &vct[2].x, &vct[2].y);
					vct[2].z = 0;
				}
				else {
					if (c2 == c1 + 1) {
						sscanf(point, "%d//%d", &vct[2].x, &vct[2].z);
						vct[2].y = 0;
					}
					else {
						sscanf(point, "%d/%d/%d", &vct[2].x, &vct[2].y, &vct[2].z);
					}

				}
			}

			//printf("%d %d %d %d %d %d %d %d %d\n", vct[0].x, vct[0].y, vct[0].z, vct[1].x, vct[1].y, vct[1].z, vct[2].x, vct[2].y, vct[2].z);
			insertArrayf(&(*model).f, vct);
		}
		else {
			// Found something else. Maybe a comment?
		}
	}
}

// Draws a model
void drawModel(Model * model) {
	int i, a, b, c;
	glBegin(GL_TRIANGLES);
		for (i = 0; i < model->f.used; i++) {
			// First point
			a = model->f.array1[i].x - 1; // Index of vertice
			b = model->f.array1[i].y - 1; // Index of texture coordonates
			c = model->f.array1[i].z - 1; // Index of normal
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

			// Second point
			a = model->f.array2[i].x - 1;
			b = model->f.array2[i].y - 1;
			c = model->f.array2[i].z - 1;
			if (c >= 0) {
				glNormal3f(model->vn.array[c].x, model->vn.array[c].y, model->vn.array[c].z);
			}
			if (b >= 0) {
				glTexCoord2f(model->vt.array[b].x, model->vt.array[b].y);
			}
			if (a >= 0) {
				glVertex3f(model->v.array[a].x, model->v.array[a].y, model->v.array[a].z);
			}

			// Third point
			a = model->f.array3[i].x - 1;
			b = model->f.array3[i].y - 1;
			c = model->f.array3[i].z - 1;
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
	glEnd();
}