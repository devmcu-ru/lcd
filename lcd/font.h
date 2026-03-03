#pragma once

#include <stddef.h>
#include <stdint.h>


typedef struct __attribute__((packed)) {
  uint16_t              first;
  uint16_t              last;
  uint8_t               height;
  uint8_t               spacing;
} LCDFontHeader;


typedef struct __attribute__((packed)) {
  uint8_t               width;
  uint8_t               left;
  uint16_t              offset;
} LCDFontLookup;


typedef struct {
  const LCDFontHeader   header;
  const uint8_t         data[];
} LCDFont;


size_t font_text_width(const LCDFont* font, const char* text);