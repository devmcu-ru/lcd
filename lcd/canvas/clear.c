#include "../canvas.h"


void canvas_clear(Canvas* canvas)
{
  canvas->lcd->driver.clear(canvas->lcd);
}
