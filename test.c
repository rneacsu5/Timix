#include <stdio.h>
#include <string.h>

typedef struct _info {
	int age;
	char school[32];
} Info;

typedef struct _myData {
	int size;
	float ratio;
	char nume[23];
	Info info;
} MyData;

MyData loadFromFile(char *filename)
{
	MyData data;
	FILE *fp = fopen(filename, "rb");
	fread(&data, sizeof(MyData), 1, fp);
	fclose(fp);
	return data;
}

void saveToFile(char *filename, MyData data)
{
	FILE *fp = fopen(filename, "wb");
	fwrite(&data, sizeof(MyData), 1, fp);
	fclose(fp);
}

int main(int argc, char const *argv[])
{
	MyData data = loadFromFile("myData.dat");

	printf("%s\n", data.nume);
	printf("%s\n", data.info.school);

	FILE *fp = fopen("alex.txt", "rt");
	int a;
	float b;
	char c[32];
	fscanf(fp, "%d\n%f %s", &a, &b, c);
	fclose(fp);

	printf("%s\n", c);
	printf("%d\n", a);
	printf("%f\n", b);

	return 0;
}