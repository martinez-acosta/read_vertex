#include "cmdline.h"
#include "definiciones.h"
#include "draw_lines.h"
#include <errno.h>
#include <math.h>
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

  for (struct vector *original = file->vertexes; original != NULL;
       original = original->next, copy = copy->next) {
    copy->x = original->x;
    copy->y = original->y;
    copy->z = original->z;
    copy->w = original->w;
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

      for (y = line->p.y; y <= line->q.y; y+=5) {
        interpolated.y = y;
        generate_frame(file, &interpolated);
      }
    }
  }
  return 0;
}
