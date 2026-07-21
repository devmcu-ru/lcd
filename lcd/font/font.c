#include "../font.h"


size_t font_text_width(const LCDFont* font, const char* text)
{
  // Проверяем текст
  if (!text) return 0;

  // Проверяем шрифт
  if (!font) return 0;
  const LCDFontHeader *header = &font->header;

  // Пробем между символами
  const uint8_t spacing = header->spacing;

  // Таблица переходов
  const uint16_t first = header->first;
  const uint16_t last = header->last;
  const LCDFontLookup *lookup = (const LCDFontLookup *) (const void *) font->data;

  // Определяем ширину строки
  size_t width = 0;
  while (*text) {

    // Считываем символ
    const uint8_t c = (uint8_t) *(text ++);

    // Перенос строки
    if (c == '\n') {
      width = 0;
      continue;
    }

    // Проверяем символ
    if (c < first || c > last) continue;

    // Добавляем ширину символа
    const uint8_t idx = c - first;
    width += lookup[idx].width + lookup[idx].left + spacing;
  }

  // Возвращем ширину текста
  return width ? width - spacing : 0;
}


uint8_t font_char_width(const LCDFont* font, char ch)
{
  // Проверяем шрифт
  if (!font) return 0;
  const LCDFontHeader *header = &font->header;

  // Проверяем символ
  const uint8_t c =  (uint8_t) ch;
  const uint16_t first = header->first;
  if (c < first || c > header->last) return 0;

  // Определяем ширину символа
  const uint8_t idx = c - first;
  const LCDFontLookup *lookup = (const LCDFontLookup *) (const void *) font->data;
  return lookup[idx].width + lookup[idx].left;
}
