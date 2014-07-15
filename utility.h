typedef struct _vector {
	GLfloat x, y, z;
} vector;

void normalizev(vector *v);
void multiplyv(vector *v, GLfloat factor);
vector addv(vector v1, vector v2);
vector substractv(vector v1, vector v2);
vector rotatev(vector v, GLfloat a);
vector createv(GLfloat x, GLfloat y, GLfloat z);