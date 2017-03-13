#include "cmdline.h"
#include "definiciones.h"
#include "draw_lines.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init(struct gengetopt_args_info *args_info, struct objfile *file);
void get_vectors_and_faces(struct objfile *file);
void read_vertex(char *line, struct vector *v);
void read_face(char *line, struct face *w);
void normalize(struct objfile *file);
void prepare_framebuffer(struct frame *image);
void viewport_transformation(struct objfile *file, float res_x, float res_y);

void init(struct gengetopt_args_info *args_info, struct objfile *file) {

  // Si no hay nombre de un archivo de entrada
  if (!args_info->input_given || !args_info->output_given)
    error("Especifique un nombre de archivo");

  // Obtenemos...
  // Nombre de archivo de entrada
  strcpy(file->inputfile, args_info->input_arg);

  // Nombre de archivo de salida
  strcpy(file->outputfile, args_info->output_arg);
  // Resolución de las imágenes
  char *str_tmp;

  // Resolución horizontal
  str_tmp = strtok(args_info->resolution_arg, ",");

  if (!str_tmp)
    error("Error al obtener en resolución horizontal");

  file->image->res_x = strtol(str_tmp, (char **)NULL, 10);

  // Resolución vertical
  str_tmp = strtok(NULL, "\n\r\v\f");

  if (!str_tmp)
    error("Error al obtener resolución vertical");

  file->image->res_y = strtol(str_tmp, (char **)NULL, 10);
}

void translate_to(struct objfile *file, struct vector *p) {
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
  for (struct vector *tmp = file->vertexes; tmp != NULL; tmp = tmp->next) {
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

void normalize(struct objfile *file) {
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

void prepare_framebuffer(struct frame *image) {
  // Reservamos la memoria que usaremos
  image->buffer = malloc(image->res_x * image->res_y * sizeof(int));

  if (!image->buffer)
    fatal("Error from function malloc() in prepare_framebuffer()",
          strerror(errno));

  // Creamos un fondo blanco en el framebuffer
  memset(image->buffer, 255, image->res_x * image->res_y * sizeof(int));
}

void viewport_transformation(struct objfile *file, float res_x, float res_y) {
  int i, j, v = 0;

  float_matrix *M = &file->M;
  float vector[4];
  float vector_tmp[4];

  // limpiamos estructuras
  memset(M, 0, sizeof(float) * 4 * 4);
  memset(&vector_tmp, 0, sizeof(float) * 4);

  // Trasladamos a la región de interés
  (*M)[0][0] = 1;
  (*M)[1][1] = 1;
  (*M)[2][2] = 1;
  (*M)[3][3] = 1;
  (*M)[0][3] = 960; // res_x / 2;
  (*M)[1][3] = 540; // res_y / 2;

  printf("M[3][0]:%f\n", (*M)[3][0]);
  printf("M[3][1]:%f\n", (*M)[3][1]);

  // asignamos valores en la matriz
  /*(*M)[0][0] = (x - 0) / 2;
  (*M)[1][1] = (y - 0) / 2;
  (*M)[2][2] = 1/2;
  (*M)[0][3] = (x - 0) / 2;
  (*M)[1][3] = (y - 0) / 2;
  (*M)[2][3] = 1 / 2;
  (*M)[3][3] = 1;
*/
  // Por cada vértice que haya
  for (struct vector *tmp = file->vertexes; tmp != NULL; tmp = tmp->next) {
    // Asignamos vector a transformar
    vector[0] = tmp->x;
    vector[1] = tmp->y;
    vector[2] = tmp->z;
    vector[3] = tmp->w;

    // Multiplicamos la matriz por el vector (Ax)
    for (j = 0; j < 4; j++)
      for (i = 0; i < 4; i++)
        vector_tmp[j] += (*M)[j][i] * vector[i];

    // vector_tmp[0] = (*M)[0][0] * vector[0] + (*M)[1][0] * vector[0] +
    //             (*M)[2][0] * vector[2] + (*M)[3][0] * vector[3];
    /*for (i = 0; i < 4; i++)
      printf("vector[%d]: %f ", i, vector_tmp[i]);
    printf("\n");*/
    v++;
    // Asignamos nuevos valores a los vértices
    tmp->x = vector_tmp[0];
    tmp->y = vector_tmp[1];
    tmp->z = vector_tmp[2];
    tmp->w = vector_tmp[3];

    // limpiamos vector
    memset(&vector_tmp, 0, sizeof(float) * 4);
  }
  // Escalamos
  (*M)[0][0] = 2 / 960;
  (*M)[1][1] = 2 / 540;
  (*M)[2][2] = 1;
  (*M)[3][3] = 1;

  // Por cada vértice que haya
  for (struct vector *tmp = file->vertexes; tmp != NULL; tmp = tmp->next) {
    // Asignamos vector a transformar
    vector[0] = tmp->x;
    vector[1] = tmp->y;
    vector[2] = tmp->z;
    vector[3] = tmp->w;

    // Multiplicamos la matriz por el vector (Ax)
    for (j = 0; j < 4; j++)
      for (i = 0; i < 4; i++)
        vector_tmp[j] += (*M)[j][i] * vector[i];

    v++;
    // Asignamos nuevos valores a los vértices
    tmp->x = vector_tmp[0];
    tmp->y = vector_tmp[1];
    tmp->z = vector_tmp[2];
    tmp->w = vector_tmp[3];

    // limpiamos vector
    memset(&vector_tmp, 0, sizeof(float) * 4);
  }
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

void translate_to_origin(struct vector *vertexes, float t) {
  for (struct vector *tmp = vertexes; tmp != NULL; tmp = tmp->next) {
    tmp->x = tmp->x + t;
    tmp->y = tmp->y + t;
    tmp->z = tmp->z + t;
  }
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
  char *face_1, *face_2, *face_3, *tmp;
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
  struct vector *tmp_vertex, *last_vertex;
  struct face *tmp_face, *last_face;
  float tmp_smallest;

  // Iniciamos la lista de vectores y caras del objeto
  file->vertexes = malloc(sizeof(struct vector));
  file->faces = malloc(sizeof(struct face));

  if (!file->vertexes || !file->faces)
    fatal("Error from malloc() in get_vectors_and_faces()", strerror(errno));

  // Asignamos los primeros elementos de nuestra lista enlazada file->vertexes,
  file->first_vector = tmp_vertex = file->vertexes;
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
      tmp_vertex->next = malloc(sizeof(struct vector));

      if (!tmp_vertex->next)
        fatal("Error from malloc(struct vertex) in get_info_file()",
              strerror(errno));
      last_vertex = tmp_vertex;
      tmp_vertex = tmp_vertex->next;
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

void print_info(struct objfile *file) {
  int i = 0;

  for (struct vector *v = file->vertexes; v != NULL; v = v->next)
    printf("vertex[%d]: (%f, %f, %f, %f)\n", i++, v->x, v->y, v->z, v->w);
  i = 0;
  for (struct face *v = file->faces; v != NULL; v = v->next)
    printf("face[%d]: (%d, %d, %d)\n", i++, v->v1, v->v2, v->v3);
}
void rasterize(frame *im, char *filename) {
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

int main(int argc, char *argv[]) {
  struct gengetopt_args_info args_info;
  struct objfile *file;

  // Inicializamos función para obtener datos de entrada
  if (cmdline_parser(argc, argv, &args_info))
    fatal("Error from cmdline_parse() in main()", strerror(errno));

  // Inicializamos estructuras
  file = malloc(sizeof(struct objfile));

  memset(file, 0, sizeof(struct objfile));

  // Si es NULL el apuntador
  if (!file)
    fatal("Error from malloc() in main()", strerror(errno));

  file->image = malloc(sizeof(struct frame));

  memset(file->image, 0, sizeof(struct frame));

  if (!file->image)
    fatal("Error from malloc() in main()", strerror(errno));

  // Obtenemos datos de entrada
  init(&args_info, file);

  // Obtenemos vectores y caras del objeto
  get_vectors_and_faces(file);

  // Calculamos las coordenadas del objeto
  get_object_coordinates(file);

  // Normalizamos el objeto al espacio acotado por el cubo unitario
  normalize(file);

  // Recalculamos las nuevas coordenadas de objeto; debe haber solo vectores
  // dentro del cubo unitario
  get_object_coordinates(file);

  // Realizamos todas las transformaciones que queramos:
  // rotación, traslación,escalamiento; todas acotadas en el cubo unitario

  // Terminadas las transformaciones, trasladamos a espacio de imagen (Viewport
  // transformation)
  struct vector r1, r2;
  r1 = {500,500,1,1};
  r2 = {1000,1000,1,1};
  viewport_transformation(file, 1000, 1000);

  // Recalculamos las nuevas coordenadas de objeto; debe haber solo vértices
  // dentro del rango de la imagen
  get_object_coordinates(file);

  // Preparamos el framebuffer
  prepare_framebuffer(file->image);

  struct vector *p0, *p1, *p2, tmp;

  // Dibujamos los segmentos de línea que definen a cada triángulo (cara)
  for (struct face *f = file->faces; f != NULL; f = f->next) {
    // Obtenemos los vértices
    p0 = get_vector(f->v1, file->vertexes);
    p1 = get_vector(f->v2, file->vertexes);
    p2 = get_vector(f->v3, file->vertexes);

    // Dibujamos a pares los vectores
    // p0 con p1
    bresenham_line(round(p0->x), round(p0->y), round(p1->x), round(p1->y),
                   file->image->buffer, file->image->res_x, file->image->res_y);
    // p1 con p2
    bresenham_line(round(p1->x), round(p1->y), round(p2->x), round(p2->y),
                   file->image->buffer, file->image->res_x, file->image->res_y);
    // p2 con p0
    bresenham_line(round(p2->x), round(p2->y), round(p0->x), round(p0->y),
                   file->image->buffer, file->image->res_x, file->image->res_y);
  }

  // Rasterizamos el buffer
  rasterize(file->image, file->outputfile);

  return 0;
}
