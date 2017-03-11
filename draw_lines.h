#ifndef RASTER_LINES_H
#define RASTER_LINES_H
void bresenham_line(int x0, int y0, int x1, int y1, int *framebuffer, int res_x,
                    int res_y);
static void translate_to();
static void swap(int *a, int *b);
#endif // RASTER_LINES_H
