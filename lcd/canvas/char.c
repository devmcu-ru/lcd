#include "../canvas.h"


size_t canvas_char(Canvas* canvas, int x, int y, const unsigned char c)
{
  // Проверяем шрифт
  const LCDFont *font = canvas->font;
  if (!font) return 0;

  // Проверяем символ
  const LCDFontHeader *header = &font->header;
  const uint16_t first = header->first;
  const uint16_t last = header->last;
  const uint8_t ch = (uint8_t) c;
  if (ch < first || ch > last) return 0;

  // Данные символа
  const size_t index = ch - first;
  const LCDFontLookup *lookup = ((const LCDFontLookup *) (const void *) font->data) + index;
  const uint16_t offset = lookup->offset;
  const uint8_t *data = ((const uint8_t *) (const void *) font) + offset;

  // Ширина
  const uint8_t width = lookup->width;
  const uint8_t left = lookup->left;
  if (width < 1) return left;

  // Рисуем символ
  const uint8_t height = header->height;
  for (uint8_t w = 0; w < width; w ++) {
    const int cx = x + (int) w + left;
    for (uint16_t n = 0; n < height;) {

      // Считываем сегмент
      uint8_t span = *(data ++);

      // Размер данных
      uint8_t remain = 8;
      if (n + remain > height) remain = (uint8_t) (height - n);

      // Нет данных
      if (!span) { n += remain; continue; }

      // Рисуем вертикальный сегмент
      while (remain) {
        // Пропускаем пустые биты
        if (!(span & 0x01)) {
          remain --;
          n ++;
          span >>= 1;
          continue;
        }
        // Определяем размер сегмента
        uint8_t tail = 0;
        while (remain && (span & 0x01)) {
          tail ++;
          span >>= 1;
          remain --;
        }
        // Рисуем сегмент
        const int cy = y + (int) n;
        if (tail == 1) canvas_pixel(canvas, cx, cy);
        else canvas_fill(canvas, cx, cy, cx, cy - 1 + (int) tail);
        n += tail;
      }
    }
  }

  // Возвращаем ширину символа
  return (size_t) width + left;
}
