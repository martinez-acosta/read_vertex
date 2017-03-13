#include "draw_lines.h"

typedef struct point {
  int x;
  int y;
} point;

static void translate_to(struct point *p, struct point *q,
                         const struct point offset);
static octant_point to_first_octant(struct point *p, struct point *q);

static void translate_to(struct point *p, struct point *q,
                         const struct point offset) {
  p->x -= offset.x;
  p->y -= offset.y;

  q->x -= offset.x;
  q->y -= offset.y;
}
static octant_point to_first_octanct(struct point *p, struct point *q) {
  enum octant_point octant = first_octant;
  const struct point delta { p->x - q->x, p->y - q->y };
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

  return octant;
}

static void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

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
  // Si el punto final, q,  es igual a res_x o res_y
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

  // Implementamos el algoritmo Bresenham

  // Trasladamos el segmento de línea a que empiece en el origen y tenga una
  // pendiente 0 <= m <= 1
  const enum octant_point octant = to_first_octant(p, q);
  const struct point delta = {abs(p.x - q.x), abs(p.y - q.y)};
  struct point tmp_point;
  int o = 2 * (delta.y - delta.x);
  int x, y;
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
    // Guardamos el punto en el framebuffer
    data[res_x * interpolated.y + interpolated.x] =
        (r << 16) | (g << 8) | (b & 0xff);
  }
}
