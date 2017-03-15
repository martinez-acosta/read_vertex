#include "definiciones.h"
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

void swap(float *a, float *b) {
  float tmp = *a;
  *a = *b;
  *b = tmp;
}

void fatal(char *str, char *error) {
  printf("%s: %s\n", str, error);
  exit(1);
}

void error(char *str) {
  puts(str);
  exit(1);
}
float degree_to_rad(float degree) { return (degree * M_PI) / 180.0; }

float smallest_float(const float first, const float second, const float third) {
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

float greatest_float(const float first, const float second, const float third) {
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

struct vector *get_vector(int p, struct vector *vertexes) {
  for (int i = 0; i < p - 1; i++)
    vertexes = vertexes->next;
  return vertexes;
}
void normalize_tmp(struct objfile *file) {
  get_object_coordinates_tmp(file);
  struct vector *min, *max;
  min = &file->obj_coordinates_tmp.min;
  max = &file->obj_coordinates_tmp.max;

  // Obtenemos el valor más grande y el más chico
  float greatest = greatest_float(max->x, max->y, max->z);
  float smallest = smallest_float(min->x, min->y, min->z);

  float max_float;

  // Vemos cuál valor absoluto es mayor para normalizar el objeto
  if (fabsf(smallest) > fabsf(greatest))
    max_float = fabsf(smallest);
  else
    max_float = fabsf(greatest);

  for (struct vector *tmp = file->tmp_vertexes; tmp != NULL; tmp = tmp->next) {
    tmp->x /= max_float;
    tmp->y /= max_float;
    tmp->z /= max_float;
  }
}
void normalize(struct objfile *file) {
  get_object_coordinates(file);
  struct vector *min, *max;
  min = &file->obj_coordinates.min;
  max = &file->obj_coordinates.max;

  // Obtenemos el valor más grande y el más chico
  float greatest = greatest_float(max->x, max->y, max->z);
  float smallest = smallest_float(min->x, min->y, min->z);

  float max_float;

  // Vemos cuál valor absoluto es mayor para normalizar el objeto
  if (fabsf(smallest) > fabsf(greatest))
    max_float = fabsf(smallest);
  else
    max_float = fabsf(greatest);

  for (struct vector *tmp = file->vertexes; tmp != NULL; tmp = tmp->next) {
    tmp->x /= max_float;
    tmp->y /= max_float;
    tmp->z /= max_float;
  }
}

void do_matrix_multiplication(float_matrix *M, struct vector *vertexes) {
  int i, j;
  float vector[4];
  float vector_tmp[4];

  // Por cada vértice que haya
  for (struct vector *tmp = vertexes; tmp != NULL; tmp = tmp->next) {
    // Asignamos vector a transformar
    vector[0] = tmp->x;
    vector[1] = tmp->y;
    vector[2] = tmp->z;
    vector[3] = tmp->w;

    // Multiplicamos la matriz por el vector (Ax)
    for (j = 0; j < 4; j++)
      for (i = 0; i < 4; i++)
        vector_tmp[j] += (*M)[j][i] * vector[i];

    // Asignamos nuevos valores a los vértices
    tmp->x = vector_tmp[0];
    tmp->y = vector_tmp[1];
    tmp->z = vector_tmp[2];
    tmp->w = vector_tmp[3];

    // limpiamos vector
    memset(&vector_tmp, 0, sizeof(float) * 4);
  }
}

void viewport_transformation(struct screen_coordinates view,
                             struct vector *vertexes) {
  float tmp_matrix[4][4];
  float vector_tmp[4];

  // Limpiamos
  memset(&tmp_matrix, 0, sizeof(float) * 4 * 4);
  memset(&vector_tmp, 0, sizeof(float) * 4);

  // Asignamos valores de matriz de traslación
  tmp_matrix[0][0] = (view.pf.x - view.po.x) / 2;
  tmp_matrix[1][1] = (view.pf.y - view.po.y) / 2;
  tmp_matrix[2][2] = 1/2;
  tmp_matrix[0][3] = (view.pf.x + view.po.x) / 2;
  tmp_matrix[1][3] = (view.pf.y + view.po.y) / 2;
  tmp_matrix[2][3] = 1/2;
  tmp_matrix[3][3] = 1;

  do_matrix_multiplication(&tmp_matrix, vertexes);
}

void translate_transform(struct vector translate, struct vector *vertexes) {
  float tmp_matrix[4][4];
  float vector_tmp[4];

  // Limpiamos
  memset(&tmp_matrix, 0, sizeof(float) * 4 * 4);
  memset(&vector_tmp, 0, sizeof(float) * 4);

  // Asignamos valores de matriz de traslación
  tmp_matrix[0][0] = 1;
  tmp_matrix[1][1] = 1;
  tmp_matrix[2][2] = 1;
  tmp_matrix[0][3] = translate.x;
  tmp_matrix[1][3] = translate.y;
  tmp_matrix[2][3] = translate.z;
  tmp_matrix[3][3] = 1;
  do_matrix_multiplication(&tmp_matrix, vertexes);
}

void scale_transform(struct vector scale, struct vector *vertexes) {
  float tmp_matrix[4][4];
  float vector_tmp[4];

  // Limpiamos
  memset(&tmp_matrix, 0, sizeof(float) * 4 * 4);
  memset(&vector_tmp, 0, sizeof(float) * 4);

  // Asignamos valores de matriz de traslación
  tmp_matrix[0][0] = scale.x;
  tmp_matrix[1][1] = scale.y;
  tmp_matrix[2][2] = scale.z;
  do_matrix_multiplication(&tmp_matrix, vertexes);
}

void rotation_transform_x(float beta, struct vector *vertexes) {
  float tmp_matrix[4][4];
  float vector_tmp[4];
  int alpha = (int)beta;
  // Limpiamos
  memset(&tmp_matrix, 0, sizeof(float_matrix));
  memset(&vector_tmp, 0, sizeof(float) * 4);

  // Rotación alrededor de x
  if (alpha != 0) {
    tmp_matrix[0][0] = 1;
    tmp_matrix[1][1] = cosf(beta);
    tmp_matrix[1][2] = -1 * sinf(beta);
    tmp_matrix[2][1] = sinf(beta);
    tmp_matrix[2][2] = cosf(beta);
    tmp_matrix[3][3] = 1;
    do_matrix_multiplication(&tmp_matrix, vertexes);
    memset(&tmp_matrix, 0, sizeof(float_matrix));
  }
}

void rotation_transform_z(float beta, struct vector *vertexes) {
  float tmp_matrix[4][4];
  float vector_tmp[4];
  int alpha = (int)beta;
  // Limpiamos
  memset(&tmp_matrix, 0, sizeof(float_matrix));
  memset(&vector_tmp, 0, sizeof(float) * 4);

  // Rotación alrededor de z
  if (alpha != 0) {
    tmp_matrix[0][0] = cosf(beta);
    tmp_matrix[0][1] = -1 * sinf(beta);
    tmp_matrix[1][0] = sinf(beta);
    tmp_matrix[1][1] = cosf(beta);
    tmp_matrix[2][2] = 1;
    tmp_matrix[3][3] = 1;
    do_matrix_multiplication(&tmp_matrix, vertexes);
    memset(&tmp_matrix, 0, sizeof(float_matrix));
  }
}

void rotation_transform_y(float beta, struct vector *vertexes) {
  float tmp_matrix[4][4];
  float vector_tmp[4];
  int alpha = (int)beta;

  // Limpiamos
  memset(&tmp_matrix, 0, sizeof(float_matrix));
  memset(&vector_tmp, 0, sizeof(float) * 4);

  // Rotación alrededor de y
  if (alpha != 0) {
    tmp_matrix[0][0] = cosf(beta);
    tmp_matrix[0][2] = sinf(beta);
    tmp_matrix[1][1] = 1;
    tmp_matrix[2][0] = -1 * sinf(beta);
    tmp_matrix[2][2] = cosf(beta);
    tmp_matrix[3][3] = 1;
    do_matrix_multiplication(&tmp_matrix, vertexes);
    memset(&tmp_matrix, 0, sizeof(float_matrix));
  }
}
void reflection_transform_x(struct vector *vertexes) {
  for (struct vector *tmp = vertexes; tmp != NULL; tmp = tmp->next) {
    tmp->x *= -1;
  }
}

void reflection_transform_y(struct vector *vertexes) {
  float tmp_matrix[4][4];
  float vector_tmp[4];

  // Limpiamos
  memset(&tmp_matrix, 0, sizeof(float_matrix));
  memset(&vector_tmp, 0, sizeof(float) * 4);

  tmp_matrix[0][0] = 1;
  tmp_matrix[1][1] = -1;
  tmp_matrix[2][2] = 1;
  tmp_matrix[3][3] = 1;

  do_matrix_multiplication(&tmp_matrix, vertexes);
}

void reflection_transform_z(struct vector *vertexes) {
  float tmp_matrix[4][4];
  float vector_tmp[4];

  // Limpiamos
  memset(&tmp_matrix, 0, sizeof(float_matrix));
  memset(&vector_tmp, 0, sizeof(float) * 4);

  tmp_matrix[0][0] = 1;
  tmp_matrix[1][1] = 1;
  tmp_matrix[2][2] = -1;
  tmp_matrix[3][3] = 1;

  do_matrix_multiplication(&tmp_matrix, vertexes);
}

void get_object_coordinates_tmp(struct objfile *file) {
  struct vector min;
  struct vector max;

  struct vector *tmp = file->tmp_vertexes;

  // Asignamos valores iniciales
  min.x = max.x = tmp->x;
  min.y = max.y = tmp->y;
  min.z = max.z = tmp->z;
  min.w = max.w = tmp->w;

  // Recorremos todos los vectores que definen al objeto buscando la 4-tupla
  // mayor y menor
  tmp = tmp->next;

  for (; tmp != NULL; tmp = tmp->next) {

    if (tmp->x < min.x)
      min.x = tmp->x;
    if (tmp->x > max.x)
      max.x = tmp->x;

    if (tmp->y < min.y)
      min.y = tmp->y;
    if (tmp->y > max.y)
      max.y = tmp->y;

    if (tmp->z < min.z)
      min.z = tmp->z;
    if (tmp->z > max.z)
      max.z = tmp->z;

    if (!tmp->next)
      break;
  }

  file->obj_coordinates_tmp.min.x = min.x;
  file->obj_coordinates_tmp.min.y = min.y;
  file->obj_coordinates_tmp.min.z = min.z;
  file->obj_coordinates_tmp.min.w = min.w;

  file->obj_coordinates_tmp.max.x = max.x;
  file->obj_coordinates_tmp.max.y = max.y;
  file->obj_coordinates_tmp.max.z = max.z;
  file->obj_coordinates_tmp.max.w = max.w;
}

void get_object_coordinates(struct objfile *file) {
  struct vector min;
  struct vector max;

  struct vector *tmp = file->vertexes;

  // Asignamos valores iniciales
  min.x = max.x = tmp->x;
  min.y = max.y = tmp->y;
  min.z = max.z = tmp->z;
  min.w = max.w = tmp->w;

  // Recorremos todos los vectores que definen al objeto buscando la 4-tupla
  // mayor y menor
  tmp = tmp->next;

  for (; tmp != NULL; tmp = tmp->next) {

    if (tmp->x < min.x)
      min.x = tmp->x;
    if (tmp->x > max.x)
      max.x = tmp->x;

    if (tmp->y < min.y)
      min.y = tmp->y;
    if (tmp->y > max.y)
      max.y = tmp->y;

    if (tmp->z < min.z)
      min.z = tmp->z;
    if (tmp->z > max.z)
      max.z = tmp->z;

    if (!tmp->next)
      break;
  }

  file->obj_coordinates.min.x = min.x;
  file->obj_coordinates.min.y = min.y;
  file->obj_coordinates.min.z = min.z;
  file->obj_coordinates.min.w = min.w;

  file->obj_coordinates.max.x = max.x;
  file->obj_coordinates.max.y = max.y;
  file->obj_coordinates.max.z = max.z;
  file->obj_coordinates.max.w = max.w;
}

void read_vertex(char *line, struct vector *v) {
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
  char *face_1, *face_2, *face_3;
  // Dividimos la línea en tres segmentos
  strtok(line, " ");
  face_1 = strtok(NULL, " ");
  face_2 = strtok(NULL, " ");
  face_3 = strtok(NULL, " ");

  // Procesamos cada cara

  if (strstr(face_1, "/"))
    w->v1 = strtol(strtok(face_1, "/"), (char **)NULL, 10);
  else
    w->v1 = strtol(strtok(face_1, "\n\r\v\f"), (char **)NULL, 10);

  if (strstr(face_2, "/"))
    w->v2 = strtol(strtok(face_2, "/"), (char **)NULL, 10);
  else
    w->v2 = strtol(strtok(face_2, "\n\r\v\f"), (char **)NULL, 10);

  if (strstr(face_3, "/"))
    w->v3 = strtol(strtok(face_3, "/\n\r\v\f"), (char **)NULL, 10);
  else
    w->v3 = strtol(strtok(face_3, "\n\r\v\f"), (char **)NULL, 10);
}

void get_vectors_and_faces(struct objfile *file) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  struct vector *tmp_vertex, *last_vertex, *tmp_vertexes;
  struct face *tmp_face, *last_face;

  // Iniciamos la lista de vectores y caras del objeto
  file->vertexes = malloc(sizeof(struct vector));
  file->tmp_vertexes = malloc(sizeof(struct vector));
  file->faces = malloc(sizeof(struct face));

  if (!file->vertexes || !file->faces || !file->tmp_vertexes)
    fatal("Error from malloc() in get_vectors_and_faces()", strerror(errno));

  // Asignamos los primeros elementos de nuestra lista enlazada file->vertexes,
  file->first_vector = tmp_vertex = file->vertexes;
  tmp_vertexes = file->tmp_vertexes;
  tmp_face = file->faces;

  // Abrimos el archivo
  fp = fopen(file->inputfile, "r");

  if (!fp)
    fatal("Error from fopen() in get_vectors_and_faces()", strerror(errno));

  // Leemos línea por línea el archivo
  while ((read = getline(&line, &len, fp)) != -1) {

    // Obtenemos vértices
    if (*(line + 0) == 'v' && *(line + 1) == ' ') {
      read_vertex(line, tmp_vertex);
      file->n_vectors++;
      // Realizamos copia del vector
      tmp_vertexes->x = tmp_vertex->x;
      tmp_vertexes->y = tmp_vertex->y;
      tmp_vertexes->z = tmp_vertex->z;
      tmp_vertexes->w = tmp_vertex->w;

      tmp_vertex->next = malloc(sizeof(struct vector));
      tmp_vertexes->next = malloc(sizeof(struct vector));

      if (!tmp_vertex->next || !tmp_vertexes->next)
        fatal("Error from malloc(struct vertex) in get_info_file()",
              strerror(errno));

      last_vertex = tmp_vertex;
      tmp_vertex = tmp_vertex->next;
      tmp_vertexes = tmp_vertexes->next;
    }

    // Obtenemos caras
    if (*(line + 0) == 'f' && *(line + 1) == ' ') {
      read_face(line, tmp_face);
      file->n_faces++;
      tmp_face->next = malloc(sizeof(struct face));
      if (!tmp_face->next)
        fatal("Error from malloc(struct vertex_face) in get_info_file()",
              strerror(errno));
      last_face = tmp_face;
      tmp_face = tmp_face->next;
    }
  }
  // Indicamos fin de las listas simples
  tmp_vertexes->next = NULL;
  free(tmp_face);
  free(tmp_vertex);
  last_face->next = NULL;
  last_vertex->next = NULL;
  file->last_vector = last_vertex;
  // Cerrramos archivo
  fclose(fp);

  // Liberamos memoria de line
  if (line)
    free(line);
}

void get_lines(struct objfile *file, char *input_lines) {
  file->lines = malloc(sizeof(struct line_segment));

  if (!file->lines)
    fatal("Error from get_lines()", strerror(errno));

  // Limpiamos estructura
  memset(file->lines, 0, sizeof(line_segment));

  char *tmp, buf[20];
  char a, b, c;
  struct line_segment *tmp_line = file->lines;
  memset(&buf, 0, sizeof(buf));
  // Si solo hay un segmento de línea
  if (!strstr(input_lines, "/")) {
    sscanf(input_lines, "%d%c%d%c%d%c%d", &tmp_line->p.x, &a, &tmp_line->p.y,
           &b, &tmp_line->q.x, &c, &tmp_line->q.y);
  } else {

    tmp = strtok(input_lines, "/");
    // Asignamos dirección inicial de la lista
    tmp_line = file->lines;
    // Obtenemos segmentos de línea, trabajaremos con la copia local buf
    while (tmp != NULL) {
      sscanf(tmp, "%d%c%d%c%d%c%d", &tmp_line->p.x, &a, &tmp_line->p.y, &b,
             &tmp_line->q.x, &c, &tmp_line->q.y);
      // Alojamos memoria
      tmp_line->next = malloc(sizeof(struct line_segment));

      if (!tmp_line->next)
        fatal("error from malloc()", strerror(errno));

      tmp_line = tmp_line->next;
      tmp = strtok(NULL, "/\0");
    }
  }
}

void print_info(struct objfile *file) {
  int i = 0;

  for (struct vector *v = file->vertexes; v != NULL; v = v->next)
    printf("vertex[%d]: (%f, %f, %f, %f)\n", i++, v->x, v->y, v->z, v->w);
  i = 0;
  for (struct face *v = file->faces; v != NULL; v = v->next)
    printf("face[%d]: (%d, %d, %d)\n", i++, v->v1, v->v2, v->v3);
}

void rasterize(frame *im, char *filename, char *output_dir) {
  char outfile[255];
  int i, j, r, g, b;
  int *data = im->buffer;

  im->fp = fopen(filename, "w");

  if (!im->fp)
    fatal("Error from fopen in line_raster()", strerror(errno));

  // Indicamos que es una imagen RGB
  fprintf(im->fp, "P3\n");

  // resolución
  fprintf(im->fp, "%d ", im->res_x);
  fprintf(im->fp, "%d \n", im->res_y);

  // máximo valor de un color
  fprintf(im->fp, "255\n");

  // guardamos imagen
  for (j = 0; j < im->res_y; j++)
    for (i = 0; i < im->res_x; i++) {
      r = (data[im->res_x * j + i] >> 16) & 0xff;
      g = (data[im->res_x * j + i] >> 8) & 0xff;
      b = (data[im->res_x * j + i] >> 0) & 0xff;

      fprintf(im->fp, "%d %d %d ", r, g, b);
    }
  fclose(im->fp);
}
