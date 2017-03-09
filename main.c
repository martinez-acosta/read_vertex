#include "cmdline.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct objfile {
  char filename[255];      // nombre del archivo
  struct face *faces;      // caras del objfile
  struct vertex *vertexes; // vértices del archivo objfile
  float smallest;          // vértice más negativo
} objfile;

typedef struct point {
  int x;
  int y;
  int z;
} point;

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
  struct vertex_face *tmp_vertex_face;

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
      loat(tmp_vertex->x, tmp_vertex->y, tmp_vertex->z);

      // Calculamos el menor número si hay alguno número menor a tmp_smallest
      if ((tmp_vertex->x < tmp_smallest) || (tmp_vertex->y < tmp_smallest) ||
          (tmp_vertex->z < tmp_smallest))
        tmp_smallest =
            smallest_float(tmp_vertex->x, tmp_vertex->y, tmp_vertex->z);

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
  // Asignamos el flotante menor si es menor a cero
  if (tmp_smallest < 0)
    file->smallest = tmp_smallest;

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
  for (struct vertex_face *v = file->faces; v != NULL; v = v->next)
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

  if (cmdline_parser(argc, argv, &args_info))
    read_vertex_fatal("Error from cmdline_parse() in main()", strerror(errno));

  if (!args_info.input_given)
    read_vertex_error("Especifique un nombre de archivo");

  // Limpiamos estructura a usar
  memset(&file, 0, sizeof(file));

  // Copiamos el nombre de archivo
  strcpy(file.filename, args_info.input_arg);

  // Obtenemos vértices y caras
  get_vertexes_and_faces(&file);
  // Imprimimos vértices y caras
  // print_info(&file);

  // Si hay coordenadas negativas, trasladamos los vértices al origen
  if (file.smallest < 0)
    translate_to_origin(file.vertexes, file.smallest);

  // Trasladar al origen
  // Escalar
  // Trasladar a las coordenadas de imagen

  struct point p, q;
  struct vertex *p0, *p1, *p2;
  int j;

  // calculamos los extremos de cada línea a dibujar en la imagen
  for (struct face *f = file.faces; f != NULL; f = f->next) {
    for (j = 0; j < 3; j++) {
      // obtenemos los vértices
      p0 = get_vertex(f->v1, file.vertexes);
      p1 = get_vertex(f->v2, file.vertexes);
      p2 = get_vertex(f->v3, file.vertexes);
      // trasladamos
      // escalamos
      // los procesamos a pares
      // p0 con p1
      // p1 con p2
      // p2 con p3

      printf("pixeles: (%d,%d) (%d,%d)", x0, y0, x1, y1);
    }
  }
  return 0;
}
