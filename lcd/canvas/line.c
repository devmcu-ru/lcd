#include <stdbool.h>
#include <stddef.h>
#include "../canvas.h"


#define SWAP(a, b)  { typeof(a) tmp = (a); (a) = (b); (b) = tmp; }


static inline void draw_span(Canvas *c, bool swap, int y, int x1, int x2)
{
  if (x1 == x2) canvas_pixel(c, swap ? y : x1, swap ? x1 : y);
  else if (swap) canvas_fill(c, y, x1, y, x2);
  else canvas_fill(c, x1, y, x2, y);
}


void canvas_line(Canvas* canvas, int x1, int y1, int x2, int y2)
{
  // Ширина и высота линии
  int dx = x2 > x1 ? x2 - x1 : x1 - x2;
  int dy = y2 > y1 ? y2 - y1 : y1 - y2;

  // Прямая линия
  if (!dx || !dy) {
    canvas_fill(canvas, x1, y1, x2, y2);
    return;
  }

  // Если функция растет быстрее по оси Y,
  // то меняем координаты местами
  bool swap = (dy > dx);
  if (swap) {
    SWAP(dx, dy);
    SWAP(x1, y1);
    SWAP(x2, y2);
  }

  // Если функция убывающая, то меняем координаты местами
  if (x1 > x2) {
    SWAP(x1, x2);
    SWAP(y1, y2);
  }

  // Накопленная ошибка
  dx = x2 - x1;
  int error = dx >> 1;

  // Направление роста функции по Y
  int step = (y1 < y2) ? 1 : -1;

  // Отрисовываем график
  int px = x1;
  int y = y1;
  for (int x = x1; x < x2; x ++) {

    // Накапливаем ошибку
    error -= dy;

    // Шаг по оси Y
    if (error < 0) {

      // Рисуем сегмент вдоль вспомогательной оси Y
      draw_span(canvas, swap, y, px, x);
      px = x + 1;

      // Сдвигаемся по оси Y и накапливаем ошибку
      y += step;
      error += dx;
    }
  }

  // Остался небольшой кусочек линии, отрисовываем
  if (px <= x2) draw_span(canvas, swap, y, px, x2);
}
