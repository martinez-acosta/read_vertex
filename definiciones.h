#ifndef DEFINICIONES_H
#define DEFINICIONES_H
#include <stdio.h>

// Matriz a usar para las transformaciones
typedef float float_matrix[4][4];

//  Todo vector en R3 lo definimos por una 4-tupla ordenada (x,y,z,w) donde
//  x,y,z indica su posición; w su posición en espacio homogeneo
typedef struct vector {
  float x;             // Posición en x
  float y;             // Posición en y
  float z;             // Posición en z
  float w;             // Posición homogenea
  struct vector *next; // Apuntador al siguiente elemento
} vector;

// Polígino definido por tres enteros que indican su posición en la lista donde
// se guardan todos los vectores que definen el objeto

typedef struct face {
  int v1; //  posición en la lista del primer vector
  int v2; //  posición en la lista del segundo vector
  int v3; //  posición en la lista del vercer vector
  struct face *next;
} face;

typedef struct object_coordinates {
  struct vector min;
  struct vector max;
} object_coordinates;

typedef struct frame {
  FILE *fp;    //  Apuntador al archivo de salida
  int *buffer; //  Framebuffer a trabajar
  int res_x;   //  Resolución horizontal del frame
  int res_y;   //  Resolución vertical del frame
} frame;

typedef struct objfile {
  char inputfile[255];  //  Nombre del archivo de entrada
  char outputfile[255]; // Nombre del archivo de salida
  struct face *faces;   // Conjunto de caras(polígonos) que definen el objeto
  struct vector *vertexes; // Vértices del objeto
  float_matrix M[4][4];    // Matriz a usar
  struct frame *image;
  struct object_coordinates obj_coordinates;
  int n_vectors; // Número de vectores que definen al objeto
  int n_faces;   // Número de caras que definen al objeto
} objfile;

// Interfaz funciones

void fatal(char *str, char *error);
void error(char *str);
void swap(float *a, float *b);
float smallest_float(const float a, const float b, const float c);
float greatest_float(const float a, const float b, const float c);
struct vector *get_vector(int p, struct vector *vertexes);
#endif // DEFINICIONES_H
