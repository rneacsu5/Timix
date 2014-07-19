#define PI 3.14159265359
#define DEG_TO_RAD PI / 180
#define RAD_TO_DEG 180 / PI

typedef struct _vector {
	GLfloat x, y, z;
} vector;

void normalizev(vector *v);
void multiplyv(vector *v, GLfloat factor);
vector addv(vector v1, vector v2);
vector substractv(vector v1, vector v2);
vector rotatev(vector v, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
vector createv(GLfloat x, GLfloat y, GLfloat z);
GLfloat vlength(vector v);
void printv(vector v);