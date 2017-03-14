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

typedef struct screen_coordinates {
  struct vector po; // Punto inicial
  struct vector pf; // Punto final
} screen_coordinates;

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
  struct vector *first_vector;
  struct vector *last_vector;
  float_matrix M[4][4]; // Matriz a usar
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
void init(struct gengetopt_args_info *args_info, struct objfile *file);
void get_vectors_and_faces(struct objfile *file);
void read_vertex(char *line, struct vector *v);
void read_face(char *line, struct face *w);
void normalize(struct objfile *file);
void prepare_framebuffer(struct frame *image);
void get_object_coordinates(struct objfile *file);
void viewport_transformation(struct screen_coordinates viewport,
                             struct vector *vertexes);
void translate_transform(struct vector translate, struct vector *vertexes);
void scale_transform(struct vector scale, struct vector *vertexes);
void rotation_transform_x(float beta, struct vector *vertexes);
void rotation_transform_y(float beta, struct vector *vertexes);
void rotation_transform_z(float beta, struct vector *vertexes);
void reflection_transform_x(struct vector *vertexes);
void reflection_transform_y(struct vector *vertexes);
void reflection_transform_z(struct vector *vertexes);
void rasterize(frame *im, char *filename);
void do_matrix_multiplication(float_matrix *M, struct vector *vertexes);

#endif // DEFINICIONES_H
