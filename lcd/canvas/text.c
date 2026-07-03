#include "../canvas.h"


void canvas_text(Canvas* canvas, int x, int y, const unsigned char* text)
{
  // Проверяем текст
  if (!text) return;

  // Шрифт
  const LCDFont *font = canvas->font;
  if (!font) return;

  // Пробел между символами
  const uint8_t spacing = font->header.spacing;

  // Запоминаем начальное значение X
  int text_x = x;

  // Рисуем текст, пока позволяем ширина экрана
  while (*text) {

    // Считываем символ
    const char c = *(text ++);

    // Новая строка
    if (c == '\n') {
      y += (int) font->header.height;
      text_x = x;
      continue;
    }

    // Начало строки
    if (c == '\r') {
      text_x = x;
      continue;
    }

    // Отрисовываем символ и смещаемся к следующей позиции
    text_x += canvas_char(canvas, text_x, y, c) + spacing;
  }
}
