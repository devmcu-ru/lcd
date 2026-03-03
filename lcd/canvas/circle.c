#include "../canvas.h"


void canvas_circle(Canvas* canvas, int xc, int yc, int r)
{
  int x = 0;
  int y = r;
  int d = 1 - r;

  while (x <= y) {
    canvas_pixel(canvas, xc + x, yc + y);
    canvas_pixel(canvas, xc - x, yc + y);
    canvas_pixel(canvas, xc + x, yc - y);
    canvas_pixel(canvas, xc - x, yc - y);
    canvas_pixel(canvas, xc + y, yc + x);
    canvas_pixel(canvas, xc - y, yc + x);
    canvas_pixel(canvas, xc + y, yc - x);
    canvas_pixel(canvas, xc - y, yc - x);
    if (d < 0) d += 2 * x + 3;
    else {
        d += 2 * (x - y) + 5;
        y--;
    }
    x ++;
  }
}
