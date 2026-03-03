#include "../canvas.h"


#define SWAP(a, b) { typeof(a) tmp = a; a = b; b = tmp; }


void canvas_rectangle(Canvas* canvas, int x1, int y1, int x2, int y2)
{
  if (y1 > y2) SWAP(y1, y2);
  canvas_line(canvas, x1, y1, x2, y1);
  canvas_line(canvas, x1, y2, x2, y2);
  canvas_line(canvas, x1, y1 + 1, x1, y2 - 1);
  canvas_line(canvas, x2, y1 + 1, x2, y2 - 1);
}
