#include "../canvas.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


void canvas_fill(Canvas* c, int x1, int y1, int x2, int y2)
{
  const LcdDriver *driver = &c->lcd->driver;

  // Область исключения не задана, рисуем как есть
  if (!c->clip.active) {
    driver->fill(c->lcd, x1, y1, x2, y2);
    return;
  }

  // Область заливки лежит за пределами области исключения
  const CanvasRectangle clip = c->clip.rect;
  const int rx1 = MIN(x1, x2);
  const int ry1 = MIN(y1, y2);
  const int rx2 = MAX(x1, x2);
  const int ry2 = MAX(y1, y2);
  if (
      rx1 > clip.b.x || rx2 < clip.a.x ||
      ry1 > clip.b.y || ry2 < clip.a.y
  ) {
    driver->fill(c->lcd, x1, y1, x2, y2);
    return;
  }

  // Верхняя часть
  if (ry1 < clip.a.y) driver->fill(c->lcd, rx1, ry1, rx2, clip.a.y - 1);

  // Нижняя часть
  if (ry2 > clip.b.y) driver->fill(c->lcd, rx1, clip.b.y + 1, rx2, ry2);

  // Левая и правая часть
  const int xy1 = MAX(clip.a.y, ry1);
  const int xy2 = MIN(clip.b.y, ry2);
  if (xy1 <= xy2) {
    if (rx1 < clip.a.x) driver->fill(c->lcd, rx1, xy1, clip.a.x - 1, xy2);
    if (rx2 > clip.b.x) driver->fill(c->lcd, clip.b.x + 1, xy1, rx2, xy2);
  }
}
