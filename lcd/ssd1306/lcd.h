#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../lcd.h"


#define SSD1306_BUFFER_SIZE 128U


typedef enum {
  SSD1306_STREAM_COMMAND,
  SSD1306_STREAM_DATA,
  SSD1306_STREAM_IDLE
} SSD1306Stream;

typedef enum {
  SSD1306_PIXEL_WHITE,
  SSD1306_PIXEL_BLACK
} SSD1306Pixel;


typedef struct {
  uint8_t           buffer1[SSD1306_BUFFER_SIZE];
  uint8_t           buffer2[SSD1306_BUFFER_SIZE];
  uint8_t           *active;
  size_t            size;
} SSD1306Buffer;


typedef struct {
  uint8_t           x1;
  uint8_t           y1;
  uint8_t           x2;
  uint8_t           y2;
} SSD1306UpdateArea;


typedef struct {
  Lcd               base;
  void              (*delay_ms)(uint32_t ms);
  void              (*i2c_wait)();
  void              (*i2c_transmit)(uint8_t *tx, size_t len);
  SSD1306Stream     stream;
  SSD1306Pixel      pixel;
  uint8_t           *frame;
  SSD1306Buffer     buffer;
  SSD1306UpdateArea update;
} LcdSSD1306;


void ssd1306_init(LcdSSD1306 *lcd);
void ssd1306_begin(LcdSSD1306 *ssd1306, SSD1306Stream stream);
void ssd1306_write(LcdSSD1306 *ssd1306, uint8_t data);
void ssd1306_commit(LcdSSD1306 *ssd1306);
void ssd1306_flush(LcdSSD1306 *lcd);
