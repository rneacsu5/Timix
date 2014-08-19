#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include "../include/bitmap.h"
#include "../include/lobjder.h"

char * modelsPath = NULL;
char * materialsPath = NULL;
char * texturesPath = NULL;
Material defaultMaterial;

// Initializes the Array and allocates memory
void initArrayv(Arrayv *a, size_t initialSize) {
	a->array = (vector3f *) malloc(initialSize * sizeof(vector3f));
	a->used = 0;
	a->size = initialSize;
	if (initialSize <= 0)
		a->size = 1;
}

// Adds an element to the array and reallocates memory if needed
void insertArrayv(Arrayv *a, vector3f element) {
	if (a->used == a->size) {
		a->size *= 2;
		a->array = (vector3f *) realloc(a->array, a->size * sizeof(vector3f));
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
	if (initialSize <= 0)
		a->size = 1;
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
	if (initialSize <= 0)
		a->size = 1;
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

void initArraymi(Arraymi *a, size_t initialSize) {
	a->array = (unsigned int *) malloc(initialSize * sizeof(unsigned int));
	a->used = 0;
	a->size = initialSize;
	if (initialSize <= 0)
		a->size = 1;
}

void insertArraymi(Arraymi *a, unsigned int mat) {
	if (a->used == a->size) {
		a->size *= 2;
		a->array = (unsigned int *) realloc(a->array, a->size * sizeof(unsigned int));
	}
	a->array[a->used++] = mat;
}

void freeArraymi(Arraymi *a) {
	free(a->array);
	a->array = NULL;
	a->used = a->size = 0;
}

// Sets paths to look for files
void setPaths(char * modelsFolderPath, char * materialsFolderPath, char * texturesFolderPath) {
	modelsPath = modelsFolderPath;
	materialsPath = materialsFolderPath;
	texturesPath = texturesFolderPath;
}

// Loads a .obj file to a model
void loadOBJToModel(char * fileName, Model * model) {
	// Opens file in read-text mode
	char * path;
	if (modelsPath != NULL) {
		path = (char *) malloc((strlen(modelsPath) + strlen(fileName) + 1) * sizeof(char));
		path[0] = '\0';
		strcat(path, modelsPath);
		strcat(path, fileName);
	}
	else {
		path = (char *) malloc((strlen(fileName) + 1) * sizeof(char));
		path[0] = '\0';
		strcat(path, fileName);
	}
	FILE * fp = fopen(path, "rt");
	if (fp == NULL) {
		printf("WARNING: Failed to open \"%s\".\n", path);
		return;
	}

	// Initializes the model's arrays
	initArrayv(&model->v, 10);
	initArrayv(&model->vt, 10);
	initArrayv(&model->vn, 10);	
	initArrayf(&model->f, 10);
	initArraymi(&model->matsi, 10);
	initArraym(&model->mats, 10);

	// Now we need to read the file line by line
	char * line = (char *) malloc(128 * sizeof(char));
	int matIndex = 0;
	vector3f vect;
	while (fgets(line, 128, fp)) {
		// The v flag is a vertice
		if (strncmp(line, "v ", 2) == 0) {
			if (sscanf(line, "%*s %f %f %f", &vect.x, &vect.y, &vect.z) == 3)
				insertArrayv(&model->v, vect);
		}

		// The vt flag is a texture coordonate
		else if (strncmp(line, "vt", 2) == 0) {
			vect.z = 0;
			if (sscanf(line, "%*s %f %f", &vect.x, &vect.y) == 2)
				insertArrayv(&model->vt, vect);
		}

		// The vn flag is a normal
		else if (strncmp(line, "vn", 2) == 0) {
			if (sscanf(line, "%*s %f %f %f", &vect.x, &vect.y, &vect.z) == 3)
				insertArrayv(&(*model).vn, vect);
		}

		// The f flag is a face
		else if (strncmp(line, "f ", 2) == 0) {
			vector3i vct[4]; // Output vector
			char * point = NULL; // A vertice/texture/normal sequence

			// Note that there are four formats:
			// <1>     f 145 679 12
			// <2>     f 145/32 679/45 12/64
			// <3>     f 145/32/457 679/45/4799 12/64/146
			// <4>     f 145//457 679//4799 12//146

			// We need to analyze each one

			int i;
			char * p[5]; // Pointers to blank spaces
			int fail = 0; // Used to detect failures in reading line
			p[0] = strchr(line, 32);
			for (i = 0; i < 4 ; i++) {
				if (p[i] == NULL) {
					if (i <= 2) {
						fail++;
					}
					else if (i == 3){
						vct[3] = vct[2];
					}
					free(point);
					break;
				}
				p[i + 1] = strchr(p[i] + 1, 32);
				if (p[i + 1] != NULL) {
					free(point);
					point = (char *) malloc(p[i + 1] - p[i]);
					strncpy(point, p[i] + 1, p[i + 1] - p[i] - 1); // Take a sequence
					point[p[i + 1] - p[i] - 1] = '\0'; // Add the null character
				}
				else {
					free(point);
					point = (char *) malloc(strlen(p[i] + 1) + 1);
					strcpy(point, p[i] + 1);
				}
				char * c1; // Pointer to first forward-slash
				char * c2; // Pointer to second forward-slash
				// These two poiters can be NULL so this is how we figure out the format used

				c1 = strchr(point, '/'); // Check for any forward-slash
				if (c1 == NULL) {
					// None found. This is format <1>
					if (sscanf(point, "%d", &vct[i].x) != 1)
						fail++;
					vct[i].y = vct[i].z = 0;
				}
				else {
					// Found the first one. Lets check for the second
					c2 = strchr(c1 + 1, '/');
					if (c2 == NULL) {
						// Second one is missing. Format <2>
						if (sscanf(point, "%d/%d", &vct[i].x, &vct[i].y) != 2)
							fail++;
						vct[i].z = 0;
					}
					else {
						// Found the second one
						// Let's check if they are next to each other
						if (c2 == c1 + 1) {
							// Format <4>
							if (sscanf(point, "%d//%d", &vct[i].x, &vct[i].z) != 2)
								fail++;
							vct[i].y = 0;
						}
						else {
							// Format <3>
							if (sscanf(point, "%d/%d/%d", &vct[i].x, &vct[i].y, &vct[i].z) != 3)
								fail++;
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
			if (fail == 0) {
				insertArrayf(&(*model).f, vct);
				insertArraymi(&(*model).matsi, matIndex);
			}
		}
		// .mtl file reference
		else if (strncmp(line, "mtllib", 6) == 0){
			char * name = (char *) malloc(128 * sizeof(char));
			if (sscanf(line, "%*s %s", name) == 1)
				loadMTLToMaterials(name, &model->mats, 0);
		}
		// The usemtl flag is a material
		else if (strncmp(line, "usemtl", 6) == 0) {
			char * name = (char *) malloc(128 * sizeof(char));
			int i;
			if (sscanf(line, "%*s %s", name) == 1) {
				for (i = 0; i < model->mats.used; i++) {
					if (strcmp(name, model->mats.array[i].matName) == 0) {
						matIndex = i;
						break;
					}
				}
			}
		}
		else {
			// Found something else. Maybe a comment?
		}
	}

	// Print Stats
	printf("MODEL:\n");
	printf("Loaded \"%s\".\n", path);
	printf("Contained: %zd vertice, %zd texture coord, %zd normals, %zd faces, %zd materials\n", 
			model->v.used, 
			model->vt.used, 
			model->vn.used, 
			model->f.used,
			model->mats.used);
	printf("Allocated %zd bytes of memory\n", 
			model->v.size * sizeof(vector3f) + 
			model->vt.size * sizeof(vector3f) + 
			model->vn.size * sizeof(vector3f) + 
			model->f.size * sizeof(vector3i) * 4 +
			model->mats.size * sizeof(Material) +
			model->matsi.size * sizeof(unsigned int));
	printf("\n");

	// Free memory
	free(line);
	free(path);

	// Close file
	fclose(fp);
}


void loadMTLToMaterials(char * fileName, Arraym * mats, int init) {
	// Opens file in read-text mode
	char * path;
	if (materialsPath != NULL) {
		path = (char *) malloc((strlen(materialsPath) + strlen(fileName) + 1) * sizeof(char));
		path[0] = '\0';
		strcat(path, materialsPath);
		strcat(path, fileName);
	}
	else {
		path = (char *) malloc((strlen(fileName) + 1) * sizeof(char));
		path[0] = '\0';
		strcat(path, fileName);
	}
	FILE * fp = fopen(path, "rt");
	if (fp == NULL) {
		printf("WARNING: Failed to open \"%s\".\n", path);
		return;
	}
	// Initialize the array if init != 0
	if (init != 0) {
		initArraym(mats, 1);
	}

	// Now we need to read the file line by line
	int i = 0;
	Material mat;
	char * line = (char *) malloc(128 * sizeof(char));
	while (fgets(line, 128, fp)) {
		// The newmtl flag is a new material
		if (strncmp(line, "newmtl", 6) == 0) {
			if (i != 0) {
				insertArraym(mats, mat);
			}
			i++;
			loadDefaultMaterial();
			mat = defaultMaterial;
			sscanf(line, "%*s %s", mat.matName);
			glGenTextures(1, &mat.glTexName);
			glBindTexture(GL_TEXTURE_2D, mat.glTexName);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		// Material ambient
		else if (strncmp(line, "Ka", 2) == 0) {
			sscanf(line, "%*s %f %f %f", &mat.Ka[0], &mat.Ka[1], &mat.Ka[2]);
		}
		// Material diffuse
		else if (strncmp(line, "Kd", 2) == 0) {
			sscanf(line, "%*s %f %f %f", &mat.Kd[0], &mat.Kd[1], &mat.Kd[2]);
		}
		// Material specular
		else if (strncmp(line, "Ks", 2) == 0) {
			sscanf(line, "%*s %f %f %f", &mat.Ks[0], &mat.Ks[1], &mat.Ks[2]);
		}
		// Material shininess
		else if (strncmp(line, "Ns", 2) == 0) {
			sscanf(line, "%*s %f", &mat.Ns);
		}
		// Material transparency
		else if (strncmp(line, "Tr", 2) == 0 || strncmp(line, "d ", 2) == 0) {
			sscanf(line, "%*s %f", &mat.Tr);
		}
		// Material illum
		else if (strncmp(line, "illum", 5) == 0) {
			sscanf(line, "%*s %d", &mat.illum);
		}
		// Texture file
		else if (strncmp(line, "map_Kd", 6) == 0) {
			// There are 2 options that we need to check: -o and -s
			char * p;
			p = strstr(line, "-o");
			if (p != NULL) {
				// Found -o option. This is texture offset
				sscanf(p, "%*s %f %f", &mat.offset.x, &mat.offset.y);
			}
			p = strstr(line, "-s");
			if (p != NULL) {
				// Found -s option. This is texture scale
				sscanf(p, "%*s %f %f", &mat.scale.x, &mat.scale.y);
			}
			// The file name is the last
			p = strchr(line, 32);
			char * p1 = strchr(p + 1, 32);
			while (p1 != NULL) {
				p = p1;
				p1 = strchr(p + 1, 32);
			}
			strcpy(mat.fileName, p + 1);
			if (mat.fileName[strlen(mat.fileName) - 1] == '\n')
				mat.fileName[strlen(mat.fileName) - 1] = '\0';

			// Full path
			char * path2;
			if (texturesPath != NULL) {
				path2 = (char *) malloc((strlen(texturesPath) + strlen(mat.fileName) + 1) * sizeof(char));
				path2[0] = '\0';
				strcat(path2, texturesPath);
				strcat(path2, mat.fileName);
			}
			else {
				path2 = (char *) malloc((strlen(mat.fileName) + 1) * sizeof(char));
				path2[0] = '\0';
				strcat(path2, mat.fileName);
			}

			// Gets the texture data and texture info
			mat.texData = LoadDIBitmap(path2, &mat.texInfo);
			if (mat.texData == NULL) {
				printf("WARNING: Failed to open \"%s\".\n", path2);
			}
			else {
				// Gets texture width and height from texture info
				mat.texWidth = mat.texInfo->bmiHeader.biWidth;
				mat.texHeight = mat.texInfo->bmiHeader.biHeight;
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
							mat.texWidth, 
							mat.texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 
							mat.texData);
			}
			free(path2);
		}
		// Custom flag for pixelated textures
		else if (strncmp(line, "pixelated", 9) == 0) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else {
			// Found something else
		}
	}
	if (i != 0) {
		insertArraym(mats, mat);
	}

	// Free memory
	free(line);
	free(path);

	// Close file
	fclose(fp);

}

void loadMaterial(Material * mat) {
	GLfloat matAmbient[] = {mat->Ka[0], mat->Ka[1], mat->Ka[2], 1};
	GLfloat matDiffuse[] = {mat->Kd[0], mat->Kd[1], mat->Kd[2], 1};
	GLfloat matSpecular[] = {mat->Ks[0], mat->Ks[1], mat->Ks[2], 1};
	GLfloat matShininess[] = {mat->Ns * 128 / 1000};

	glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

	glBindTexture(GL_TEXTURE_2D, mat->glTexName);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

// Draws model using immediate mode
void drawModel(Model * model) {
	unsigned int i, j, a, b, c;
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		for (i = 0; i < model->f.used; i++) {
			if (model->mats.used > 0) {
				if (i == 0) {
					glEnd();
					loadMaterial(&model->mats.array[model->matsi.array[i]]);
					glBegin(GL_QUADS);
				}
				else if (model->matsi.array[i - 1] != model->matsi.array[i]) {
					glEnd();
					loadMaterial(&model->mats.array[model->matsi.array[i]]);
					glBegin(GL_QUADS);
				}
			}
			else if (i == 0) {
				glEnd();
				loadDefaultMaterial();
				glBegin(GL_QUADS);
			}
			for (j = 0; j < 4; j++) {
				a = model->f.array[j][i].x; // Index of vertice
				b = model->f.array[j][i].y; // Index of texture coordonates
				c = model->f.array[j][i].z; // Index of normal

				if (c > 0) {
					c--; // Substact one because the first vertice,normal, etc. starts at 1 in an .obj file
					glNormal3f(model->vn.array[c].x, model->vn.array[c].y, model->vn.array[c].z);
				}
				if (b > 0) {
					b--;
					glTexCoord2f(model->vt.array[b].x, model->vt.array[b].y);
				}
				if (a > 0) {
					a--;
					glVertex3f(model->v.array[a].x, model->v.array[a].y, model->v.array[a].z);
				}
			}
		}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void loadDefaultMaterial() {
	if (strcmp(defaultMaterial.matName, "default") != 0) {
		strcpy(defaultMaterial.matName, "default");
		strcpy(defaultMaterial.fileName, "none");
		glGenTextures(1, &defaultMaterial.glTexName);
		glBindTexture(GL_TEXTURE_2D, defaultMaterial.glTexName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		defaultMaterial.Ka[0] = defaultMaterial.Ka[1] = defaultMaterial.Ka[2] = 0.2;
		defaultMaterial.Kd[0] = defaultMaterial.Kd[1] = defaultMaterial.Kd[2] = 0.8;
		defaultMaterial.Ks[0] = defaultMaterial.Ks[1] = defaultMaterial.Ks[2] = 1.0;
		defaultMaterial.Ns = 50;
		defaultMaterial.Tr = 1;
		defaultMaterial.illum = 2;
		defaultMaterial.offset.x = defaultMaterial.offset.y = defaultMaterial.offset.z = 0;
		defaultMaterial.scale.x = defaultMaterial.scale.y = defaultMaterial.scale.z = 1;
	}
	loadMaterial(&defaultMaterial);
}