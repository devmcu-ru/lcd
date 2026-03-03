#pragma once

#include <stdint.h>
#include "color.h"


typedef struct Lcd Lcd;

typedef struct {
  void          (*color)(const Lcd *lcd, Color color);
  void          (*pixel)(const Lcd *lcd, int x, int y);
  void          (*fill)(const Lcd *lcd, int x1, int y1, int x2, int y2);
  void          (*clear)(const Lcd *lcd);
} LcdDriver;

struct Lcd {
  uint16_t      width;
  uint16_t      height;
  LcdDriver     driver;
};