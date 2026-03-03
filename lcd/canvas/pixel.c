#include "../canvas.h"


void canvas_pixel(Canvas* c, int x, int y)
{
  if (c->clip.active) {
    const CanvasRectangle clip = c->clip.rect;
    if (
      x >= clip.a.x && x <= clip.b.x &&
      y >= clip.a.y && y <= clip.b.y
    ) return;
  }
  c->lcd->driver.pixel(c->lcd, x, y);
}
