typedef float float_matrix[4][4];

typedef struct vertex {
  float x;
  float y;
  float z;
  float w;
  struct vertex *next;
} vertex;

typedef struct face {
  int v1;
  int v2;
  int v3;
  struct face *next;
} face;

typedef struct object_coordinates {
  float xmin;
  float xmax;
  float ymin;
  float ymax;
  float zmin;
  float zmax;
} object_coordinates;

typedef struct objfile {
  char inputfile[255];     // nombre del archivo de entrada
  char outputfile[255];    // nombre del archivo de salida
  struct face *faces;      // caras del objfile
  struct vertex *vertexes; // vértices del archivo objfile
  float smallest;          // vértice más negativo
  struct object_coordinates obj_coordinates;
  float_matrix M[4][4];
  int res_x;
  int res_y;
} objfile;
