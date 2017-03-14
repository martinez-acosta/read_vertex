#ifndef RASTER_LINES_H
#define RASTER_LINES_H
#include <math.h>
#include <stdlib.h>
void bresenham_line(int x0, int y0, int x1, int y1, int *framebuffer, int res_x,
                    int res_y);
void explicit_line(int x0, int y0, int x1, int y1, int *framebuffer, int res_x,
                   int res_y);
#endif // RASTER_LINES_H
