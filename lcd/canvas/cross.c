#include "../canvas.h"


void canvas_cross(Canvas* canvas, int x, int y, uint8_t size)
{
  // Размер не задан, выходим
  if (!size) return;

  // Размер должен быть нечетным
  if (!(size & 0x01)) size --;

  // Точка
  if (!size) {
    canvas_pixel(canvas, x, y);
    return;
  }

  // Маркер
  uint8_t half = size >> 1;
  canvas_line(canvas, x - half, y, x + half, y);
  canvas_line(canvas, x, y - half, x, y + half);
}
