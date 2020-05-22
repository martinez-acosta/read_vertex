#include "cmdline.h"
#include "definiciones.h"
#include "draw_lines.h"
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
char isInImage(int x, int y, int res_x, int res_y) {
  if (x > 0 && x < res_x && y > 0 && y < res_y)
    return 1;
  return 0;
}
void swap_int(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

int min_int(int a, int b, int c) {
  if (a < b && a < c)
    return a;
  else if (b < a && b < c)
    return b;
  else if (c < a && c < b)
    return c;
}

int max_int(int a, int b, int c) {
  if (a > b && a > c)
    return a;
  else if (b > a && b > c)
    return b;
  else if (c > a && c > b)
    return c;
}

float min_float(float a, float b, float c) {
  if (a < b && a < c)
    return a;
  else if (b < a && b < c)
    return b;
  else if (c < a && c < b)
    return c;
}

float max_float(float a, float b, float c) {
  if (a > b && a > c)
    return a;
  else if (b > a && b > c)
    return b;
  else if (c > a && c > b)
    return c;
}
float areaTriangle(struct vector *v1, struct vector *v2, struct vector *v3) {
  return (v3->x - v1->x) * (v2->y - v1->y) - (v3->y - v1->y) * (v2->x - v1->x);
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
    // rotation_transform_z(file->alpha, file->tmp_vertexes);
    // rotation_transform_x(file->alpha, file->tmp_vertexes);
    normalize_tmp(file);
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
void generate_frame2(struct objfile *file, struct vector *interpolated) {

  // Realizamos copia de los vértices
  struct vector min;

  if (file->rotar) {
    file->alpha += M_PI / 12;
    rotation_transform_x(file->alpha, file->vertexes);
    rotation_transform_y(file->alpha, file->vertexes);
  }

  // Preparamos el framebuffer
  prepare_framebuffer(file->image);

  struct vector p0, p1, p2;

  // Dibujamos los segmentos de línea que definen a cada triángulo (cara)
  for (struct face *f = file->faces; f != NULL; f = f->next) {
    // Obtenemos los vértices
    p0 = *get_vector(f->v1, file->vertexes);
    p1 = *get_vector(f->v2, file->vertexes);
    p2 = *get_vector(f->v3, file->vertexes);

    // trasladamos
    p0.x += interpolated->x;
    p0.y += interpolated->y;

    p1.x += interpolated->x;
    p1.y += interpolated->y;

    p2.x += interpolated->x;
    p2.y += interpolated->y;

    if ( !isInImage(p0.x,p0.y,file->image->res_x,file->image->res_y)
            && !isInImage(p1.x,p1.y,file->image->res_x,file->image->res_y)
         && !isInImage(p2.x,p2.y,file->image->res_x,file->image->res_y))
        continue;
    // Dibujamos a pares los vectores
    // p2 con p1
    bresenham_line(round(p2.x), round(p2.y), round(p1.x), round(p1.y),
                   file->image->buffer, file->image->res_x, file->image->res_y);
    // p1 con p0
    bresenham_line(round(p1.x), round(p1.y), round(p0.x), round(p0.y),
                   file->image->buffer, file->image->res_x, file->image->res_y);
    // p2 con p0
    bresenham_line(round(p2.x), round(p2.y), round(p0.x), round(p0.y),
                   file->image->buffer, file->image->res_x, file->image->res_y);
  }
  // Generamos nuevo nombre de salida
  memset(file->outputfile, 0, 255);
  sprintf(file->outputfile, "%04d", file->n_img++);
  strcat(file->outputfile, ".ppm");
  // Rasterizamos el buffer
  rasterize(file->image, file->outputfile, file->output_dir);
}
void generate_frameFaceHiding(struct objfile *file,
                              struct vector *interpolated) {

  // Realizamos copia de los vértices
  struct vector min;

  if (file->rotar) {
    file->alpha += M_PI / 12;
    rotation_transform_x(file->alpha, file->vertexes);
    rotation_transform_y(file->alpha, file->vertexes);
  }

  // Preparamos el framebuffer
  prepare_framebuffer(file->image);

  struct vector p0, p1, p2;
  struct vector v1, v2, v3, a, b, n;

  const struct vector view = {0, 0, 1};

  // Dibujamos los segmentos de línea que definen a cada triángulo (cara)
  for (struct face *f = file->faces; f != NULL; f = f->next) {
    // Obtenemos los vértices
    p0 = *get_vector(f->v1, file->vertexes);
    p1 = *get_vector(f->v2, file->vertexes);
    p2 = *get_vector(f->v3, file->vertexes);

    if ( !isInImage(p0.x+interpolated->x,p0.y+interpolated->y,file->image->res_x,file->image->res_y)
            && !isInImage(p1.x+interpolated->x,p1.y+interpolated->y,file->image->res_x,file->image->res_y)
         && !isInImage(p2.x+interpolated->x,p2.y+interpolated->y,file->image->res_x,file->image->res_y))
        continue;
    v1.x = p0.x;
    v1.y = p0.y;
    v1.z = p0.z;

    v2.x = p1.x;
    v2.y = p1.y;
    v2.z = p1.z;

    v3.x = p2.x;
    v3.y = p2.y;
    v3.z = p2.z;

    // Dos vectores que pertenecen al plano del triángulo
    a.x = v1.x - v2.x;
    a.y = v1.y - v2.y;
    a.z = v1.z - v2.z;

    b.x = v1.x - v3.x;
    b.y = v1.y - v3.y;
    b.z = v1.z - v3.z;

    // Calculamos normal

    n.x = a.y * b.z - a.z * b.y;
    n.y = a.z * b.x - a.x * b.z;
    n.z = a.x * b.y - a.y * b.x;

    if (n.x * view.x + n.y * view.y + n.z * view.z > 0) {
      // trasladamos
      p0.x += interpolated->x;
      p0.y += interpolated->y;

      p1.x += interpolated->x;
      p1.y += interpolated->y;

      p2.x += interpolated->x;
      p2.y += interpolated->y;

      // Dibujamos a pares los vectores
      // p2 con p1
      bresenham_line(round(p2.x), round(p2.y), round(p1.x), round(p1.y),
                     file->image->buffer, file->image->res_x,
                     file->image->res_y);
      // p1 con p0
      bresenham_line(round(p1.x), round(p1.y), round(p0.x), round(p0.y),
                     file->image->buffer, file->image->res_x,
                     file->image->res_y);
      // p2 con p0
      bresenham_line(round(p2.x), round(p2.y), round(p0.x), round(p0.y),
                     file->image->buffer, file->image->res_x,
                     file->image->res_y);
    }
  }
  // Generamos nuevo nombre de salida
  memset(file->outputfile, 0, 255);
  sprintf(file->outputfile, "%04d", file->n_img++);
  strcat(file->outputfile, ".ppm");
  // Rasterizamos el buffer
  rasterize(file->image, file->outputfile, file->output_dir);
}
void generate_frameFlatShading(struct objfile *file,
                               struct vector *interpolated) {

  if (file->rotar) {
    file->alpha += M_PI / 12;
    rotation_transform_x(file->alpha, file->vertexes);
    rotation_transform_y(file->alpha, file->vertexes);
  }

  // Preparamos el framebuffer
  prepare_framebuffer(file->image);

  struct vector p0, p1, p2, p;
  struct vector v1, v2, v3, a, b, n;
  struct vector min, max;
  float area, r0, g0, b0;
  float w0, w1, w2;

  const struct vector view = {0, 0, 1};
  // Colores base
  struct vector c0 = {1, 0, 0};
  struct vector c1 = {0, 1, 0};
  struct vector c2 = {0, 0, 1};
  // time_t t;
  srand(time(NULL));
  // Dibujamos los segmentos de línea que definen a cada triángulo (cara)
  for (struct face *f = file->faces; f != NULL; f = f->next) {
    // Obtenemos los vértices
    p0 = *get_vector(f->v1, file->vertexes);
    p1 = *get_vector(f->v2, file->vertexes);
    p2 = *get_vector(f->v3, file->vertexes);

    if ( !isInImage(p0.x+interpolated->x,p0.y+interpolated->y,file->image->res_x,file->image->res_y)
            && !isInImage(p1.x+interpolated->x,p1.y+interpolated->y,file->image->res_x,file->image->res_y)
         && !isInImage(p2.x+interpolated->x,p2.y+interpolated->y,file->image->res_x,file->image->res_y))
        continue;

    v1.x = p0.x;
    v1.y = p0.y;
    v1.z = p0.z;

    v2.x = p1.x;
    v2.y = p1.y;
    v2.z = p1.z;

    v3.x = p2.x;
    v3.y = p2.y;
    v3.z = p2.z;

    // Dos vectores que pertenecen al plano del triángulo
    a.x = v1.x - v2.x;
    a.y = v1.y - v2.y;
    a.z = v1.z - v2.z;

    b.x = v1.x - v3.x;
    b.y = v1.y - v3.y;
    b.z = v1.z - v3.z;

    // Calculamos normal

    n.x = a.y * b.z - a.z * b.y;
    n.y = a.z * b.x - a.x * b.z;
    n.z = a.x * b.y - a.y * b.x;

    if (n.x * view.x + n.y * view.y + n.z * view.z > 0) {

      // trasladamos
      p0.x += interpolated->x;
      p0.y += interpolated->y;

      p1.x += interpolated->x;
      p1.y += interpolated->y;

      p2.x += interpolated->x;
      p2.y += interpolated->y;

      // Obtenemos el menor de las x, y z
      min.x = min_float(p0.x, p1.x, p2.x);
      min.y = min_float(p0.y, p1.y, p2.y);

      // Obtenemos el mayor de las x,y, z
      max.x = max_float(p0.x, p1.x, p2.x);
      max.y = max_float(p0.y, p1.y, p2.y);

      // Calculamos el área del triángulo
      area = areaTriangle(&p0, &p1, &p2);
      c0.x = ((rand() % 255)) / (float)255;
      c0.y = ((rand() % 255)) / (float)255;
      c0.z = ((rand() % 255)) / (float)255;

      c1.x = ((rand() % 255)) / (float)255;
      c1.y = ((rand() % 255)) / (float)255;
      c1.z = ((rand() % 255)) / (float)255;

      c2.x = ((rand() % 255)) / (float)255;
      c2.y = ((rand() % 255)) / (float)255;
      c2.z = ((rand() % 255)) / (float)255;
      for (int y = min.y + 0.5; y < max.y + 0.5; y++)
        for (int x = min.x + 0.5; x < max.x + 0.5; x++) {
          if (!isInImage(x, y, file->image->res_x, file->image->res_y))
            continue;

          p.x = x;
          p.y = y;
          p.z = 0;
          w0 = areaTriangle(&p1, &p2, &p);
          w1 = areaTriangle(&p2, &p0, &p);
          w2 = areaTriangle(&p0, &p1, &p);
          w0 /= area;
          w1 /= area;
          w2 /= area;
          if (w0 >= 0 && w1 >= 0 && w2 >= 0) {

            r0 = w0 * c0.x + w1 * c1.x + w2 * c2.x;
            g0 = w0 * c0.y + w1 * c1.y + w2 * c2.y;
            b0 = w0 * c0.z + w1 * c1.z + w2 * c2.z;
            if (isInImage(x, y, file->image->res_x, file->image->res_y)) {
              file->image->buffer[file->image->res_x * y + x] =
                  ((int)(r0 * 255) << 16) | ((int)(g0 * 255) << 8) |
                  ((int)(b0 * 255));
            }
          }
        }
    }
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
  if (!args_info->input_given)
    error("Especifique un nombre de archivo de entrada");

  // Obtenemos...
  // Nombre de archivo de entrada
  strcpy(file->inputfile, args_info->input_arg);

  // Nombre de archivo de salida
  // strcpy(file->outputfile, args_info->output_arg);

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
  // if (args_info->line_given)
  // get_lines(file, args_info->line_arg);
  // Obtenemos curva de bézier
  if (args_info->bezier_given)
    get_bezier(file, args_info->bezier_arg);
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

  // Rotamos en grados
  rotation_transform_x(180, file->vertexes);
  rotation_transform_y(0, file->vertexes);
  rotation_transform_z(0, file->vertexes);

  // Escalamos

  if (args_info.scale_given) {
    float t = strtod(args_info.scale_arg, NULL);
    for (struct vector *tmp = file->vertexes; tmp != NULL; tmp = tmp->next) {
      tmp->x *= t;
      tmp->y *= t;
      tmp->z *= t;
    }
  }
  // Normalizamos el objeto al espacio acotado por el cubo unitario
  //  normalize2(file->vertexes);

  struct vector interpolated;
  int x, y;
  /*if (args_info.line_given) {
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
  } else*/ if (args_info.bezier_given) {

    int segments = 50;
    float t, k1, k2, k3, k4;
    struct bezier_curve *curve = file->bezier;
    for (int i = 0; i <= segments; ++i) {
      t = i / (float)segments;
      // Calculamos coeficientes
      k1 = (1 - t) * (1 - t) * (1 - t);
      k2 = 3 * (1 - t) * (1 - t) * t;
      k3 = 3 * (1 - t) * t * t;
      k4 = t * t * t;

      interpolated.x = curve->p1.x * k1 + curve->p2.x * k2 + curve->p3.x * k3 +
                       curve->p4.x * k4;

      interpolated.y = curve->p1.y * k1 + curve->p2.y * k2 + curve->p3.y * k3 +
                       curve->p4.y * k4;
      if (args_info.faceHiding_given)
        generate_frameFaceHiding(file, &interpolated);
      else if (args_info.flatShading_given)
        generate_frameFlatShading(file, &interpolated);
      else if (args_info.wireframe_given)
        generate_frame2(file, &interpolated);
    }
  }
  return 0;
}
