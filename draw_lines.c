#include "draw_lines.h"
#include <stdio.h>
// Definiciones
typedef struct point {
  int x;
  int y;
} point;

enum octant_point {
  first_octant = 0,
  second_octant = 1,
  third_octant = 2,
  four_octant = 3,
  five_octant = 4,
  six_octant = 5,
  seven_octant = 6,
  eight_octant = 7
};
// Prototipos
static void translate_to(struct point *p, struct point *q,
                         const struct point offset);
static enum octant_point to_first_octant(struct point *p, struct point *q);
static void swap(int *a, int *b);

// Implementaciones
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
    swap(&q->x, &q->y);
    break;
  case third_octant:
    q->x *= -1;
    swap(&q->x, &q->y);
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
    swap(&q->x, &q->y);
    break;
  case seven_octant:
    q->y *= -1;
    swap(&q->x, &q->y);
    break;
  case eight_octant:
    break;
  }
  return octant;
}

static void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

void bresenham_line(int x0, int y0, int x1, int y1, int *framebuffer,
                    const int res_x, const int res_y) {

  struct point p = {x0, y0};

  struct point q = {x1, y1};

  // El punto p (punto inicial) debe estar más a la izquierda que
  // el punto q.
  if (q.x < p.x) {
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
  // Si el punto final, q,  es igual a res_x o res_y
  if (p.x == res_x)
    p.x--;

  if (p.y == res_y)
    p.y--;

  if (q.x == res_x)
    q.x--;

  if (q.y == res_y)
    q.y--;

  if (p.x < res_x && q.x < res_x && p.x >= 0 && q.x >= 0 && p.y < res_y &&
      q.y < res_y && p.y >= 0 && q.y >= 0) {
    // Dibujamos punto inicial y final
    data[res_x * p.y + p.x] = (r << 16) | (g << 8) | b;
    data[res_x * q.y + q.x] = (r << 16) | (g << 8) | b;
  }

  // Si es una línea horizontal
  if (p.y == q.y) {
    for (x = p.x; x <= q.x; x++)
      if (x >= 0 && x < res_x && p.y < res_y && p.y >= 0)
        data[res_x * p.y + x] = (r << 16) | (g << 8) | b;
    return;
  }

  // Si es una línea vertical
  if (p.x == q.x) {
    if (p.y > q.y)
      swap(&p.y, &q.y);
    for (y = p.y; y <= q.y; y++)
      if (y > 0 && y <= res_y && q.x < res_x && q.x >= 0)
        data[res_x * y + q.x] = (r << 16) | (g << 8) | b;
    return;
  }

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
    tmp_point.x++;
    if (o < 0) {
      o += twoDy;
    } else {
      tmp_point.y++;
      o += twoDyDx;
    }

    interpolated = tmp_point;

    switch (octant) {
    case first_octant:
      break;
    case second_octant: // Intercambiamos las x con las y
      swap(&interpolated.x, &interpolated.y);
      break;
    case third_octant:
      swap(&interpolated.x, &interpolated.y);
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
      swap(&interpolated.x, &interpolated.y);
      interpolated.x *= -1;
      interpolated.y *= -1;
      break;
    case seven_octant:
      swap(&interpolated.x, &interpolated.y);
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
    if (interpolated.x < res_x && interpolated.y < res_y &&
        interpolated.x >= 0 && interpolated.y >= 0)
      data[res_x * interpolated.y + interpolated.x] =
          (r << 16) | (g << 8) | (b & 0xff);
  }
}
void explicit_line(int x0, int y0, int x1, int y1, int *framebuffer, int res_x,
                   int res_y) {
  float m = ((float)y1 - y0) / (x1 - x0);
  int r, g, b;
  int y;
  float k;
  int *data = framebuffer;
  r = g = b = 0;
  k = y0 - m * x0;
  for (int x = x0; x <= x1; x++) {
    y = m * x + k;
    if (x < res_x && y < res_y && x >= 0 && y >= 0)
      data[res_x * y + x] = (r << 16) | (g << 8) | (b & 0xff);
  }
}
