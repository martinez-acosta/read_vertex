#include "draw_lines.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct point {
  int x;
  int y;
} point;

static void translate_points(int *x0, int *y0, int *x1, int *y1,
                             const int x_offset, const int y_offset) {
  *x0 -= x_offset;
  *x1 -= x_offset;

  *y0 -= y_offset;
  *y1 -= y_offset;
}

static void swap(int *a, int *b) {}

// Apuntador al framebuffer a usar
void bresenham_line(int x0, int y0, int x1, int y1, int *framebuffer,
                    const int res_x, const int res_y) {

  struct point p {
    x0, y0
  };

  struct point q {
    x1, y1
  };

  // El punto p (punto inicial) debe estar más a la izquierda que
  // el punto q.
  if (q.x > p.x) {
    swap(&q.x, &p.x);
    swap(&q.y, &p.y);
  }

  // Creamos apuntador con el que procesaremos los datos
  int *data = framebuffer;

  int r, g, b;

  // Asignamos color negro
  r = g = b = 0;

  // Si es el mismo punto, lo dibujamos y salimos
  if (p.y == q.y && p.x == q.x) {
    return;
  }

  int x, y; // Valores que usaremos para hacer la interpolación

  // Solo podemos dibujar puntos en el rango {0,(N-1)} donde N = {res_x,res_y}
  // Si alguno de los puntos son iguales a res_x o res_y
  if (q.x == res_x)
    q.x--;

  if (q.y == res_y)
    q.y--;

  // Dibujamos punto inicial y final
  data[res_x * p.y + p.x] = (r << 16) | (g << 8) | b;
  data[res_x * q.y + q.x] = (r << 16) | (g << 8) | b;

  // Si es una línea horizontal
  if (p.y == q.y) {
    for (x = p.x; x <= p.x; x++)
      data[res_x * p.y + x] = (r << 16) | (g << 8) | b;
    return;
  }

  // Si es una línea vertical
  if (p.x == q.x) {
    for (p.y = y0; y <= q.y; y++)
      data[res_x * y + p.x] = (r << 16) | (g << 8) | b;
    return;
  }

  int tmp, x_offset, y_offset;
  char octant;

  //Implementamos el algoritmo Bresenham
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
