#ifndef RASTER_LINES_H
#define RASTER_LINES_H
#include <math.h>

static enum octant_point {
  first_octant = 0,
  second_octant = 1,
  third_octant = 2,
  four_octant = 3,
  five_octant = 4,
  six_octant = 5,
  seven_octant = 6,
  eight_octant = 7
};
void bresenham_line(int x0, int y0, int x1, int y1, int *framebuffer, int res_x,
                    int res_y);
static void swap(int *a, int *b);

#endif // RASTER_LINES_H
