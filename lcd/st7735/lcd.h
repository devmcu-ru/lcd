#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../lcd.h"


/* Screen size */
#define LCD_LANDSCAPE       1
#if LCD_LANDSCAPE
  #define LCD_WIDTH         160
  #define LCD_HEIGHT        128
#else
  #define LCD_WIDTH         128
  #define LCD_HEIGHT        160
#endif

/* Pixel Format */
#define LCD_PIXEL_RGB565    0x05
#define LCD_PIXEL_RGB666    0x06
#define LCD_PIXEL_FORMAT    LCD_PIXEL_RGB565

/* ST7735 registers */
#define ST7735_NOP         0x00
#define ST7735_SWRESET     0x01
#define ST7735_RDDID       0x04
#define ST7735_RDDST       0x09

#define ST7735_IDMON       0x39
#define ST7735_IDMOFF      0x38

#define ST7735_SLPIN       0x10
#define ST7735_SLPOUT      0x11
#define ST7735_PTLON       0x12
#define ST7735_NORON       0x13

#define ST7735_INVOFF      0x20
#define ST7735_INVON       0x21
#define ST7735_GAMMASET    0x26
#define ST7735_DISPOFF     0x28
#define ST7735_DISPON      0x29

#define ST7735_CASET       0x2A
#define ST7735_PASET       0x2B
#define ST7735_RAMWR       0x2C
#define ST7735_RAMRD       0x2E

#define ST7735_PTLAR       0x30
#define ST7735_MADCTL      0x36
#define ST7735_PIXFMT      0x3A
#define ST7735_RAMWRC      0x3C

#define ST7735_FRMCTR1     0xB1
#define ST7735_FRMCTR2     0xB2
#define ST7735_FRMCTR3     0xB3
#define ST7735_INVCTR      0xB4

#define ST7735_PWCTR1      0xC0
#define ST7735_PWCTR2      0xC1
#define ST7735_PWCTR3      0xC2
#define ST7735_PWCTR4      0xC3
#define ST7735_PWCTR5      0xC4
#define ST7735_VMCTR1      0xC5
#define ST7735_VMCTR2      0xC7

#define ST7735_RDID1       0xDA
#define ST7735_RDID2       0xDB
#define ST7735_RDID3       0xDC
#define ST7735_RDID4       0xDD

#define ST7735_GMCTRP1     0xE0
#define ST7735_GMCTRN1     0xE1

#define ST7735_BUFFER_SIZE 64U

typedef enum {
  ST7735_PIN_CS,
  ST7735_PIN_DC,
  ST7735_PIN_RST
} ST7735Pin;

typedef enum {
  ST7735_MODE_COMMAND,
  ST7735_MODE_DATA
} ST7735Mode;

typedef struct {
  int x1;
  int y1;
  int x2;
  int y2;
} ST7735Window;

typedef struct {
  #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB565
  uint8_t                   data[2];
  #elif LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
  uint8_t                   red;
  uint8_t                   green;
  uint8_t                   blue;
  #endif
} ST7735Pixel;

typedef struct {
  uint8_t                   buffer1[ST7735_BUFFER_SIZE];
  uint8_t                   buffer2[ST7735_BUFFER_SIZE];
  uint8_t                   *active;
  size_t                    size;
} ST7735Buffer;

typedef struct {
  Lcd                       base;
  ST7735Mode                mode;
  ST7735Pixel               pixel;
  ST7735Buffer              buffer;
  ST7735Window              window;
  void                      (*pin_set)(ST7735Pin pin);
  void                      (*pin_reset)(ST7735Pin pin);
  void                      (*delay_ms)(uint32_t ms);
  void                      (*spi_wait)();
  void                      (*spi_transmit)(uint8_t *tx, size_t len);
} LcdST7735;


void st7735_init(LcdST7735 *lcd);