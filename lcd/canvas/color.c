#include "../canvas.h"


void canvas_color(Canvas* canvas, Color color)
{
  canvas->lcd->driver.color(canvas->lcd, color);
}
