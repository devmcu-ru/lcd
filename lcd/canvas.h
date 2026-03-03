#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "./lcd.h"
#include "./font.h"


typedef struct {
  int                   x;
  int                   y;
} CanvasPoint;

typedef struct {
  CanvasPoint           a;
  CanvasPoint           b;
} CanvasRectangle;

typedef struct {
  CanvasRectangle       rect;
  bool                  active;
} CanvasClip;

typedef struct {
  const Lcd             *lcd;
  const LCDFont         *font;
  CanvasClip            clip;
} Canvas;


void canvas_init(Canvas *canvas, const Lcd *lcd);
void canvas_clear(Canvas *canvas);
void canvas_color(Canvas *canvas, Color color);
void canvas_font(Canvas *canvas, const LCDFont *font);
void canvas_clip(Canvas *canvas, const CanvasRectangle *rect);
void canvas_pixel(Canvas *canvas, int x, int y);
void canvas_fill(Canvas *canvas, int x1, int y1, int x2, int y2);
void canvas_line(Canvas *canvas, int x1, int y1, int x2, int y2);
void canvas_rectangle(Canvas *canvas, int x1, int y1, int x2, int y2);
void canvas_circle(Canvas* canvas, int xc, int yx, int r);
void canvas_arc(Canvas* canvas, int x1, int y1, int x2, int y2, int r);
void canvas_cross(Canvas *canvas, int x, int y, uint8_t size);
size_t canvas_char(Canvas *canvas, int x, int y, char c);
void canvas_text(Canvas *canvas, int x, int y, const char *text);