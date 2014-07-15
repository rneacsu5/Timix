typedef struct _vector {
	GLfloat x, y, z;
} vector;

void normalize(vector *v);
void multiply(vector *v, GLfloat factor);
vector addVectors(vector v1, vector v2);
vector substractVectors(vector v1, vector v2);
vector rotateVector(vector v, GLfloat a);
vector createVector(GLfloat x, GLfloat y, GLfloat z);