#include "cmdline.h"
#include "definiciones.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void scale_to(struct objfile *file, struct vertex *p) {}

void translate_to(struct objfile *file, struct vertex *p) {
  int i, j;

  float_matrix *M = file->M;
  float vector[4];
  float vector_tmp[4];

  // limpiamos estructuras
  memset(M, 0, sizeof(float) * 4 * 4);
  memset(&vector_tmp, 0, sizeof(float) * 4);

  // asignamos valores en la matriz
  (*M)[0][0] = 1;
  (*M)[1][1] = 1;
  (*M)[2][2] = 1;
  (*M)[0][3] = p->x;
  (*M)[1][3] = p->y;
  (*M)[2][3] = p->z;
  (*M)[3][3] = p->w;

  // Por cada vértice que haya
  for (struct vertex *tmp = file->vertexes; tmp != NULL; tmp = tmp->next) {

    memset(&vector_tmp, 0, sizeof(float) * 4);
    // Asignamos vector a transformar
    vector[0] = tmp->x;
    vector[1] = tmp->y;
    vector[2] = tmp->z;
    vector[3] = tmp->w;

    // Multiplicamos la matriz por el vector (Ax)
    for (j = 0; j < 4; j++) {
      for (i = 0; i < 4; i++) {
        vector_tmp[j] += (*M)[j][i] * vector[i];
        // printf("M[%d][%d] = %f\n", i, j, (*M)[i][j]);
      }
    }
    // Asignamos nuevos valores a los vértices
    tmp->x = vector_tmp[0];
    tmp->y = vector_tmp[1];
    tmp->z = vector_tmp[2];
    tmp->w = vector_tmp[3];
  }
}

void normalize(struct objfile *file, const float max_float) {
  for (struct vertex *tmp = file->vertexes; tmp != NULL; tmp = tmp->next) {
    tmp->x /= max_float;
    tmp->y /= max_float;
    tmp->z /= max_float;
  }
}

void viewport_transformation(struct objfile *file, int res_x, int res_y) {
  int i, j;

  float_matrix *M = file->M;
  float vector[4];
  float vector_tmp[4];

  // limpiamos estructuras
  memset(M, 0, sizeof(float) * 4 * 4);
  memset(&vector_tmp, 0, sizeof(float) * 4);

  // asignamos valores en la matriz
  (*M)[0][0] = (res_x - 0) / 2;
  (*M)[1][1] = (res_y - 0) / 2;
  (*M)[2][2] = 1 / 2;
  (*M)[0][3] = (res_x + 0) / 2;
  (*M)[1][3] = (res_y - 0) / 2;
  (*M)[2][3] = 1 / 2;
  (*M)[3][3] = 1;

  // Por cada vértice que haya
  for (struct vertex *tmp = file->vertexes; tmp != NULL; tmp = tmp->next) {

    memset(&vector_tmp, 0, sizeof(float) * 4);
    // Asignamos vector a transformar
    vector[0] = tmp->x;
    vector[1] = tmp->y;
    vector[2] = tmp->z;
    vector[3] = tmp->w;

    // Multiplicamos la matriz por el vector (Ax)
    for (j = 0; j < 4; j++) {
      for (i = 0; i < 4; i++) {
        vector_tmp[j] += (*M)[j][i] * vector[i];
        // printf("M[%d][%d] = %f\n", i, j, (*M)[i][j]);
      }
    }
    // Asignamos nuevos valores a los vértices
    tmp->x = vector_tmp[0];
    tmp->y = vector_tmp[1];
    tmp->z = vector_tmp[2];
    tmp->w = vector_tmp[3];
  }
}

void get_object_coordinates(struct objfile *file) {
  float xmin = 0;
  float xmax = 0;
  float ymin = 0;
  float ymax = 0;
  float zmin = 0;
  float zmax = 0;
  for (struct vertex *tmp = file->vertexes; tmp != NULL; tmp = tmp->next) {

    if (tmp->x < xmin)
      xmin = tmp->x;
    if (tmp->x > xmax)
      xmax = tmp->x;
    if (tmp->y < ymin)
      ymin = tmp->y;
    if (tmp->y > ymax)
      ymax = tmp->y;
    if (tmp->z < zmin)
      zmin = tmp->z;
    if (tmp->z > zmax)
      zmax = tmp->z;
  }
  file->obj_coordinates.xmin = xmin;
  file->obj_coordinates.xmax = xmax;
  file->obj_coordinates.ymin = ymin;
  file->obj_coordinates.ymax = ymax;
  file->obj_coordinates.zmin = zmin;
  file->obj_coordinates.zmax = zmax;
}

float smallest_float(float first, float second, float third) {
  float tmp;
  if (first < second && first < third) {
    tmp = first;
  } else {
    if (second < first && second < third) {
      tmp = second;
    } else {
      tmp = third;
    }
  }
  return tmp;
}

float greatest_float(float first, float second, float third) {
  float tmp;
  if (first > second && first > third) {
    tmp = first;
  } else {
    if (second > first && second > third) {
      tmp = second;
    } else {
      tmp = third;
    }
  }
  return tmp;
}

void translate_to_origin(struct vertex *vertexes, float t) {

  for (struct vertex *tmp = vertexes; tmp != NULL; tmp = tmp->next) {
    tmp->x = tmp->x + t;
    tmp->y = tmp->y + t;
    tmp->z = tmp->z + t;
  }
}
void read_vertex(char *line, struct vertex *v) {
  char *tmp;

  tmp = strtok(line, " ");

  // x
  v->x = atof(strtok(NULL, " "));
  // y
  v->y = atof(strtok(NULL, " "));
  // z
  v->z = atof(strtok(NULL, " "));
  // w; Si no hay valor para w, tomamos el valor por defecto que es 1.
  tmp = strtok(NULL, " ");
  if (tmp)
    v->w = atof(tmp);
  else
    v->w = 1;
}

void read_face(char *line, struct face *w) {
  char *tmp;
  int i;

  tmp = strtok(line, " ");

  // v1
  w->v1 = strtol(strtok(NULL, "/"), (char **)NULL, 10);
  // v2
  strtok(NULL, " ");
  w->v2 = strtol(strtok(NULL, "/"), (char **)NULL, 10);
  // v3
  strtok(NULL, " ");
  w->v3 = strtol(strtok(NULL, "/"), (char **)NULL, 10);
}

void read_vertex_fatal(char *str, char *error) {
  printf("%s: %s\n", str, error);
  exit(1);
}

void read_vertex_error(char *str) {
  puts(str);
  exit(1);
}

void get_vertexes_and_faces(struct objfile *file) {

  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  struct vertex *tmp_vertex;
  struct face *tmp_vertex_face;

  // Abrimos el archivo
  fp = fopen(file->filename, "r");

  if (!fp)
    read_vertex_fatal("Error from fopen() in main()", strerror(errno));

  // Inicializamos los vértices y caras de la estructura
  file->vertexes = malloc(sizeof(struct vertex));
  file->faces = malloc(sizeof(struct face));

  if (!file->vertexes || !file->faces)
    read_vertex_fatal("Error from malloc() in get_info_file()",
                      strerror(errno));

  tmp_vertex = file->vertexes;
  tmp_vertex_face = file->faces;

  float tmp_smallest = 0;
  // Leemos línea por línea
  while ((read = getline(&line, &len, fp)) != -1) {
    // Obtenemos vértices
    if (*(line + 0) == 'v' && *(line + 1) == ' ') {

      read_vertex(line, tmp_vertex);
      tmp_vertex->next = malloc(sizeof(struct vertex));

      if (!tmp_vertex->next)
        read_vertex_fatal("Error from malloc(struct vertex) in get_info_file()",
                          strerror(errno));
      tmp_vertex = tmp_vertex->next;
    }

    // Obtenemos caras
    if (*(line + 0) == 'f' && *(line + 1) == ' ') {
      read_face(line, tmp_vertex_face);

      tmp_vertex_face->next = malloc(sizeof(struct face));
      if (!tmp_vertex_face->next)
        read_vertex_fatal(
            "Error from malloc(struct vertex_face) in get_info_file()",
            strerror(errno));
      tmp_vertex_face = tmp_vertex_face->next;
    }
  }
  tmp_vertex_face->next = NULL;
  tmp_vertex->next = NULL;
  // Cerrramos archivo
  fclose(fp);

  if (line)
    free(line);
}

void print_info(struct objfile *file) {
  int i = 0;

  for (struct vertex *v = file->vertexes; v != NULL; v = v->next)
    printf("vertex[%d]: (%f, %f, %f, %f)\n", i++, v->x, v->y, v->z, v->w);
  i = 0;
  for (struct face *v = file->faces; v != NULL; v = v->next)
    printf("face[%d]: (%d, %d, %d)\n", i++, v->v1, v->v2, v->v3);
}

struct vertex *get_vertex(int p, struct vertex *vertexes) {
  for (int i = 0; i <= p; i++)
    vertexes = vertexes->next;
  return vertexes;
}

int main(int argc, char *argv[]) {
  struct gengetopt_args_info args_info;
  struct objfile file;

  // Obtenemos valores de entrada
  if (cmdline_parser(argc, argv, &args_info))
    read_vertex_fatal("Error from cmdline_parse() in main()", strerror(errno));

  // Si no hay nombre de un archivo
  if (!args_info.input_given)
    read_vertex_error("Especifique un nombre de archivo");

  // Limpiamos estructura a usar
  memset(&file, 0, sizeof(file));

  // Copiamos el nombre de archivo
  strcpy(file.filename, args_info.input_arg);

  // Obtenemos vértices y caras
  get_vertexes_and_faces(&file);

  // Calculamos las coordenadas del objeto
  get_object_coordinates(&file);

  // Obtenemos el valor más grande y el más chico
  float greatest =
      greatest_float(file.obj_coordinates.xmax, file.obj_coordinates.ymax,
                     file.obj_coordinates.zmax);

  float smallest =
      smallest_float(file.obj_coordinates.xmin, file.obj_coordinates.ymin,
                     file.obj_coordinates.zmin);

  float max_float;

  // Vemos cuál valor absoluto es mayor para normalizar el objeto
  if (fabsf(smallest) > fabsf(greatest)) {
    max_float = fabsf(smallest);
  } else {
    max_float = fabsf(greatest);
  }

  // Normalizamos el objeto al espacio acotado por el cubo unitario
  normalize(&file, max_float);

  // Recalculamos las nuevas coordenadas de objeto; debe haber solo vértices
  // dentro del cubo unitario
  get_object_coordinates(&file);

  // Realizamos todas las transformaciones que queramos:
  // rotación, traslación,escalamiento; todas acotadas en el cubo unitario

  // Terminadas las transformaciones, trasladamos a espacio de imagen (Viewport
  // transformation)
  file.res_x = 1920;
  file.res_y = 1080;
  viewport_transformation(&file, file.res_x, file.res_y);

  // Recalculamos las nuevas coordenadas de objeto; debe haber solo vértices dentro del rango de la imagen
  get_object_coordinates(&file);

  //Rasterizamos
  struct vertex *p0, *p1, *p2;
  int j;

  // calculamos los extremos de cada línea a dibujar en la imagen
  for (struct face *f = file.faces; f != NULL; f = f->next) {
    for (j = 0; j < 3; j++) {
      // obtenemos los vértices
      p0 = get_vertex(f->v1, file.vertexes);
      p1 = get_vertex(f->v2, file.vertexes);
      p2 = get_vertex(f->v3, file.vertexes);
      // los procesamos a pares
      // p0 con p1
      // p1 con p2
      // p2 con p3
    }
  }
  return 0;
}
