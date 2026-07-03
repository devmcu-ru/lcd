#include "stdbool.h"
#include "../canvas.h"


void canvas_scan_circle(
  Canvas* canvas,
  int xc,
  int yc,
  int r,
  CanvasScanCircleCallback callback,
  void* param
)
{
  int x = 0;
  int y = r;
  int d = 1 - r;
  bool y_changed = true;

  while (x <= y) {

    // Отрезки при изменении X
    callback(canvas, yc + x, xc - y, xc + y, param);
    if (x != 0) callback(canvas, yc - x, xc - y, xc + y, param);

    // Шагаем вдоль оси Y
    if (d < 0) d += 2 * x + 3;
    else {
      d += 2 * (x - y) + 5;
      y--;
      y_changed = true;
    }
    x ++;

    // Отрезки при изменении Y
    if (y_changed) {
      y_changed = false;
      callback(canvas, yc + y, xc - x, xc + x, param);
      if (y != 0) callback(canvas, yc - y, xc - x, xc + x, param);
    }
  }
}


static void fill_callback(Canvas *canvas, int y, int x1, int x2, void* params)
{
  if (x2 == x1) canvas_pixel(canvas, x1, y);
  else canvas_line(canvas, x1, y, x2, y);
}


void canvas_fill_circle(Canvas* canvas, int xc, int yc, int r)
{
  canvas_scan_circle(canvas, xc, yc, r, fill_callback, NULL);
}


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
