#include "cmdline.h"
#include "definiciones.h"
#include "draw_lines.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void swap_int(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

void generate_frame(struct objfile *file, struct vector *interpolated) {

  // Realizamos copia de los vértices
  struct vector *copy = file->tmp_vertexes;
  struct vector min;

  for (struct vector *original = file->vertexes; original != NULL;
       original = original->next, copy = copy->next) {
    copy->x = original->x;
    copy->y = original->y;
    copy->z = original->z;
    copy->w = original->w;
  }

  if (file->rotar) {
    normalize_tmp(file);
    min.x = file->obj_coordinates_tmp.max.x * -1;
    min.y = file->obj_coordinates_tmp.max.y * -1;
    min.z = file->obj_coordinates_tmp.max.z * -1;
    min.w = file->obj_coordinates.max.w;
    translate_transform(min, file->tmp_vertexes);

    file->alpha += M_PI / 12;
    // rotation_transform_x(M_PI, file->tmp_vertexes);
    rotation_transform_y(file->alpha, file->tmp_vertexes);
    normalize_tmp(file);
    // rotation_transform_z(file->alpha, file->tmp_vertexes);
    // rotation_transform_x(file->alpha, file->tmp_vertexes);
  }
  struct screen_coordinates view;
  view.po.x = interpolated->x;
  view.po.y = interpolated->y;
  view.pf.x = view.po.x + 600;
  view.pf.y = view.po.y + 600;
  viewport_transformation(view, file->tmp_vertexes);

  // Preparamos el framebuffer
  prepare_framebuffer(file->image);

  struct vector *p0, *p1, *p2;

  // Dibujamos los segmentos de línea que definen a cada triángulo (cara)
  for (struct face *f = file->faces; f != NULL; f = f->next) {
    // Obtenemos los vértices
    p0 = get_vector(f->v1, file->tmp_vertexes);
    p1 = get_vector(f->v2, file->tmp_vertexes);
    p2 = get_vector(f->v3, file->tmp_vertexes);

    // Dibujamos a pares los vectores
    // p2 con p1
    bresenham_line(round(p2->x), round(p2->y), round(p1->x), round(p1->y),
                   file->image->buffer, file->image->res_x, file->image->res_y);
    // p1 con p0
    bresenham_line(round(p1->x), round(p1->y), round(p0->x), round(p0->y),
                   file->image->buffer, file->image->res_x, file->image->res_y);
    // p2 con p0
    bresenham_line(round(p2->x), round(p2->y), round(p0->x), round(p0->y),
                   file->image->buffer, file->image->res_x, file->image->res_y);
  }
  // Generamos nuevo nombre de salida
  memset(file->outputfile, 0, 255);
  sprintf(file->outputfile, "%04d", file->n_img++);
  strcat(file->outputfile, ".ppm");
  // Rasterizamos el buffer
  rasterize(file->image, file->outputfile, file->output_dir);
}

static void translate_to(struct point *p, struct point *q,
                         const struct point offset) {
  p->x -= offset.x;
  p->y -= offset.y;

  q->x -= offset.x;
  q->y -= offset.y;
}
static void translate_one_point(struct point *p, const struct point offset);
static void translate_one_point(struct point *p, const struct point offset) {
  p->x -= offset.x;
  p->y -= offset.y;
}

static enum octant_point to_first_octant(struct point *p, struct point *q) {
  enum octant_point octant = first_octant;
  const struct point delta = {q->x - p->x, q->y - p->y};
  // Identificamos a qué octante pertenece el segmento de línea
  // Cada cuadrante tiene dos octantes

  // Primer cuadrante
  if (delta.x > 0 && delta.y > 0) {

    if (delta.x > delta.y)
      octant = first_octant;
    else
      octant = second_octant;
  }

  // Segundo cuadrante
  if (delta.x < 0 && delta.y > 0) {
    if (abs(delta.y) > abs(delta.x))
      octant = third_octant;
    else
      octant = four_octant;
  }

  // Tercer cuadrante
  if (delta.x < 0 && delta.y < 0) {

    if (abs(delta.x) > abs(delta.y))
      octant = five_octant;
    else
      octant = six_octant;
  }

  // Cuarto cuadrante
  if (delta.x > 0 && delta.y < 0) {

    if (abs(delta.y) > abs(delta.x))
      octant = seven_octant;
    else
      octant = eight_octant;
  }
  // Trasladamos el segmento de línea a que empiece en el origen
  translate_to(p, q, *p);
  // El vector p está en la posición (0,0,0,1)
  // Rotamos el segmento de línea al primer octante
  switch (octant) {
  case first_octant:

    break;
  case second_octant:
    swap_int(&q->x, &q->y);
    break;
  case third_octant:
    q->x *= -1;
    swap_int(&q->x, &q->y);
    break;
  case four_octant:
    q->y *= -1;
    break;
  case five_octant:
    q->y *= -1;
    q->x *= -1;
    break;
  case six_octant:
    q->y *= -1;
    q->x *= -1;
    swap_int(&q->x, &q->y);
    break;
  case seven_octant:
    q->y *= -1;
    swap_int(&q->x, &q->y);
    break;
  case eight_octant:
    break;
  }
  return octant;
}

void bresenham_interpolated(struct objfile *file, struct line_segment *line) {

  struct point p = {line->p.x, line->p.y};

  struct point q = {line->q.x, line->q.y};
  struct vector interpolated_vector;
  // El punto p (punto inicial) debe estar más a la izquierda que
  // el punto q.
  if (q.x < p.x) {
    swap_int(&q.x, &p.x);
    swap_int(&q.y, &p.y);
  }

  int x, y; // Valores que usaremos para hacer la interpolación

  // Implementamos el algoritmo Bresenham

  // Trasladamos el segmento de línea a que empiece en el origen y tenga una
  // pendiente 0 <= m <= 1
  const struct point offset = p;
  const enum octant_point octant = to_first_octant(&p, &q);
  const struct point delta = {abs(q.x - p.x), abs(q.y - p.y)};
  struct point tmp_point;
  int o = 2 * (delta.y - delta.x);
  int twoDy = 2 * delta.y;
  int twoDyDx = 2 * (delta.y - delta.x);
  struct point interpolated;
  // Asignamos el punto inicial
  tmp_point = p;

  while (tmp_point.x < q.x) {
    tmp_point.x += 5;
    if (o < 0) {
      o += twoDy;
    } else {
      tmp_point.y += 5;
      o += twoDyDx;
    }

    interpolated = tmp_point;

    switch (octant) {
    case first_octant:
      break;
    case second_octant: // Intercambiamos las x con las y
      swap_int(&interpolated.x, &interpolated.y);
      break;
    case third_octant:
      swap_int(&interpolated.x, &interpolated.y);
      interpolated.x *= -1;
      break;
    case four_octant:
      interpolated.y *= -1;
      break;
    case five_octant:
      interpolated.x *= -1;
      interpolated.y *= -1;
      break;
    case six_octant:
      swap_int(&interpolated.x, &interpolated.y);
      interpolated.x *= -1;
      interpolated.y *= -1;
      break;
    case seven_octant:
      swap_int(&interpolated.x, &interpolated.y);
      interpolated.y *= -1;
      break;
    case eight_octant:
      interpolated.y *= -1;
      break;
    }
    // Trasladamos el punto a sus coordenadas en la imagen
    translate_one_point(&interpolated,
                        (struct point){offset.x * -1, offset.y * -1});
    // Guardamos el punto en el framebuffer
    if (interpolated.x < file->image->res_x &&
        interpolated.y < file->image->res_y && interpolated.x >= 0 &&
        interpolated.y >= 0) {
      interpolated_vector.x = interpolated.x;
      interpolated_vector.y = interpolated.y;
      generate_frame(file, &interpolated_vector);
    }
  }
}
void init(struct gengetopt_args_info *args_info, struct objfile *file) {

  // Si no hay nombre de un archivo de entrada
  if (!args_info->input_given || !args_info->output_given)
    error("Especifique un nombre de archivo");

  // Obtenemos...
  // Nombre de archivo de entrada
  strcpy(file->inputfile, args_info->input_arg);

  // Nombre de archivo de salida
  strcpy(file->outputfile, args_info->output_arg);

  // Directorio de salida
  if (args_info->output_dir_given)
    strcpy(file->output_dir, args_info->output_dir_arg);

  // Resolución de las imágenes
  char *str_tmp;

  // Resolución horizontal
  str_tmp = strtok(args_info->resolution_arg, ",");

  // Rotación inicial
  if (args_info->rotate_given)
    file->rotar = true;
  if (!str_tmp)
    error("Error al obtener en resolución horizontal");

  file->image->res_x = strtol(str_tmp, (char **)NULL, 10);

  // Resolución vertical
  str_tmp = strtok(NULL, "\n\r\v\f");

  if (!str_tmp)
    error("Error al obtener resolución vertical");

  file->image->res_y = strtol(str_tmp, (char **)NULL, 10);
  // Obtenemos segmentos de líneas
  if (args_info->line_given)
    get_lines(file, args_info->line_arg);
}

void prepare_framebuffer(struct frame *image) {
  // Reservamos la memoria que usaremosoutputfile
  image->buffer = malloc(image->res_x * image->res_y * sizeof(int));

  if (!image->buffer)
    fatal("Error from function malloc() in prepare_framebuffer()",
          strerror(errno));

  // Creamos un fondo blanco en el framebuffer
  memset(image->buffer, 255, image->res_x * image->res_y * sizeof(int));
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

  // Normalizamos el objeto al espacio acotado por el cubo unitario
  normalize(file);

  // Rotamos en grados
  rotation_transform_x(M_PI, file->vertexes);
  rotation_transform_y(0, file->vertexes);
  rotation_transform_z(0, file->vertexes);

  struct vector interpolated;
  int x, y;
  // Por cada segmento de línea
  for (struct line_segment *line = file->lines; line != NULL;
       line = line->next) {

    // Si es una línea vertical
    if (line->p.x == line->q.x) {

      if (line->p.y > line->q.y)
        swap_int(&line->p.y, &line->q.y);

      interpolated.x = line->p.x;

      for (y = line->p.y; y <= line->q.y; y += 5) {
        interpolated.y = y;
        generate_frame(file, &interpolated);
      }
    } else if (line->p.y == line->q.y) { // Si es una línea horizontal

      if (line->p.x > line->q.x)
        swap_int(&line->p.x, &line->q.x);

      interpolated.x = line->p.x;

      for (x = line->p.x; y <= line->q.x; y += 5) {
        interpolated.x = y;
        generate_frame(file, &interpolated);
      }
    } else {
      bresenham_interpolated(file, line);
    }
  }
  return 0;
}
