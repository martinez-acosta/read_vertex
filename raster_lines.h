#ifndef RASTER_LINES_H
#define RASTER_LINES_H
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct point {
  int x;
  int y;
  int z;
  int w;
} point;

typedef struct frame {
  FILE *fp;
  int *buffer;
  char output[255];
  struct point p;
  struct point q;
  int rows;
  int cols;
} frame;

void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

void traslate_points(int *x0, int *y0, int *x1, int *y1, const int x_offset,
                     const int y_offset) {
  *x0 -= x_offset;
  *x1 -= x_offset;

  *y0 -= y_offset;
  *y1 -= y_offset;
}

void bresenham_line(int x0, int y0, int x1, int y1, frame *im) {
  char octant;
  int tmp, x_tmp, y_tmp, r, g, b, x, y, x_offset, y_offset;

  if (y0 == y1 && x0 == x1)
    return;

  int *data = im->buffer;

  // Rasterizamos el punto inicial
  r = g = b = 0;

  if (x0 >= im->cols)
    x0 = im->cols - 1;

  if (y0 >= im->rows)
    y0 = im->rows - 1;

  data[im->cols * y0 + x0] = (r << 16) | (g << 8) | b;

  // Rasterizamos el punto final
  if (x1 == im->cols)
    x = x1 - 1;
  else
    x = x1;

  if (y1 == im->rows)
    y = y1 - 1;
  else
    y = y1;

  data[im->cols * y + x] = (r << 16) | (g << 8) | b;

  // línea horizontal
  if (y0 == y1) {
    for (x = x0; x <= x1; x++) {
      if (x == im->cols)
        x_tmp = im->cols - 1;
      else
        x_tmp = x;
      data[im->cols * y0 + x_tmp] = (r << 16) | (g << 8) | b;
    }
    return;
  }

  // línea vertical
  if (x0 == x1) {
    for (y = y0; y <= y1; y++) {
      if (y == im->rows)
        y_tmp = im->rows - 1;
      else
        y_tmp = y;
      data[im->cols * y_tmp + x0] = (r << 16) | (g << 8) | b;
    }
    return;
  }

  int dx, dy;
  dx = x1 - x0;
  dy = y1 - y0;
  /**************************************************/
  // Por cada cuadrante tenemos dos opciones
  if (dx > 0 && dy > 0) {
    // primer cuadrante
    if (dx > dy)
      octant = 0;
    else {
      octant = 1;
      x_offset = x0;
      y_offset = y0;
      traslate_points(&x0, &y0, &x1, &y1, x_offset, y_offset);
    }
  } else if (dx < 0 && dy > 0) {
    // segundo cuadrante
    if (abs(dy) > abs(dx)) {
      octant = 2;
      x_offset = x0;
      y_offset = y0;
      traslate_points(&x0, &y0, &x1, &y1, x_offset, y_offset);
    } else {
      octant = 3;
      x_offset = x0;
      y_offset = y0;
      traslate_points(&x0, &y0, &x1, &y1, x_offset, y_offset);
    }
  } else if (dx < 0 && dy < 0) {
    // tercer cuadrante
    if (abs(dx) > abs(dy)) {
      octant = 4;
      x_offset = x0;
      y_offset = y0;
      traslate_points(&x0, &y0, &x1, &y1, x_offset, y_offset);
    } else {
      octant = 5;
      x_offset = x0;
      y_offset = y0;
      traslate_points(&x0, &y0, &x1, &y1, x_offset, y_offset);
    }
  } else if (dx > 0 && dy < 0) {
    // cuarto cuadrante
    if (abs(dy) > abs(dx)) {
      octant = 6;
      x_offset = x0;
      y_offset = y0;
      traslate_points(&x0, &y0, &x1, &y1, x_offset, y_offset);
    } else {
      octant = 7;
      x_offset = x0;
      y_offset = y0;
      traslate_points(&x0, &y0, &x1, &y1, x_offset, y_offset);
    }
  }

  switch (octant) {
  case 0:
    break;
  case 1:
    swap(&x1, &y1);
    break;
  case 2:
    x1 *= -1;
    swap(&x1, &y1);
    break;
  case 3:
    y1 *= -1;
    break;
  case 4:
    y1 *= -1;
    x1 *= -1;
    break;
  case 5:
    y1 *= -1;
    x1 *= -1;
    swap(&x1, &y1);
    break;
  case 6:
    y1 *= -1;
    swap(&x1, &y1);
    break;
  case 7:
    y1 *= -1;
    break;
  }

  // Verificamos que x0 sea menor que x1
  if (x0 > x1) {
    tmp = x1;
    x1 = x0;
    x0 = tmp;

    tmp = y1;
    y1 = y0;
    y0 = tmp;
  }

  // Aplicamos el algoritmo
  dx = abs(x1 - x0);
  dy = abs(y1 - y0);
  int p = 2 * dy - dx;
  int twoDy = 2 * dy;
  int twoDyDx = 2 * (dy - dx);
  x = x0;
  y = y0;

  while (x < x1) {
    x++;
    if (p < 0) {
      p += twoDy;
    } else {
      y++;
      p += twoDyDx;
    }

    x_tmp = x;
    y_tmp = y;

    switch (octant) {
    case 0:
      break;
    case 1:
      swap(&x_tmp, &y_tmp);
      x_tmp += x_offset;
      y_tmp += y_offset;
      break;
    case 2:
      swap(&x_tmp, &y_tmp);
      x_tmp *= -1;
      x_tmp += x_offset;
      y_tmp += y_offset;
      break;
    case 3:
      y_tmp *= -1;
      x_tmp += x_offset;
      y_tmp += y_offset;
      break;
    case 4:
      x_tmp *= -1;
      y_tmp *= -1;
      x_tmp += x_offset;
      y_tmp += y_offset;
      break;
    case 5:
      swap(&x_tmp, &y_tmp);
      x_tmp *= -1;
      y_tmp *= -1;
      x_tmp += x_offset;
      y_tmp += y_offset;
      break;
    case 6:
      swap(&x_tmp, &y_tmp);
      y_tmp *= -1;
      x_tmp += x_offset;
      y_tmp += y_offset;
      break;
    case 7:
      y_tmp *= -1;
      x_tmp += x_offset;
      y_tmp += y_offset;
      break;
    }

    // guardamos el pixel en el framebuffer

    if (x_tmp >= im->cols)
      x_tmp = im->cols - 1;

    if (y_tmp < 0)
      y_tmp = 0;

    if (y_tmp >= im->rows)
      y_tmp = im->rows - 1;

    if (y_tmp < 0)
      y_tmp = 0;
    data[im->cols * y_tmp + x_tmp] = (r << 16) | (g << 8) | (b & 0xff);
  }
}

void line_fatal(char *str, char *error) {
  printf("%s: %s\n", str, error);
  exit(1);
}
void line_error(char *str) {
  puts(str);
  exit(1);
}
void line_raster(frame *im) {
  int i, j, r, g, b;
  int *data = im->buffer;

  im->fp = fopen(im->output, "w");

  printf("Archivo de salida: %s \n", im->output);
  if (!im->fp)
    line_fatal("Error from fopen in line_raster()", strerror(errno));
  // atributos de la imagen

  // Indicamos que es una imagen RGB
  fprintf(im->fp, "P3\n");

  // resolución
  fprintf(im->fp, "%d ", im->cols);
  fprintf(im->fp, "%d \n", im->rows);

  // máximo valor de un color
  fprintf(im->fp, "255\n");

  // guardamos imagen
  for (j = 0; j < im->rows; j++)
    for (i = 0; i < im->cols; i++) {

      r = (data[im->cols * j + i] >> 16) & 0xff;
      g = (data[im->cols * j + i] >> 8) & 0xff;
      b = (data[im->cols * j + i] >> 0) & 0xff;

      fprintf(im->fp, "%d %d %d ", r, g, b);
    }
  fclose(im->fp);
}

#endif // RASTER_LINES_H
