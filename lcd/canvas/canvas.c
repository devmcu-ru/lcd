#include <stdio.h>
#include "../canvas.h"


void canvas_init(Canvas* canvas, const Lcd* lcd)
{
  canvas->lcd = lcd;
  canvas->clip.active = false;
  canvas_font(canvas, NULL);
}
