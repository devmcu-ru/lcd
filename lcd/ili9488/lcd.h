#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../lcd.h"


/* Screen size */
#define LCD_LANDSCAPE       1
#if LCD_LANDSCAPE
  #define LCD_WIDTH         480
  #define LCD_HEIGHT        320
#else
  #define LCD_WIDTH         320
  #define LCD_HEIGHT        480
#endif

/* Pixel Format */
#define LCD_PIXEL_RGB111    0x01
#define LCD_PIXEL_RGB666    0x06
#define LCD_PIXEL_FORMAT    LCD_PIXEL_RGB666

/* ILI9488 registers */
#define ILI9488_NOP         0x00
#define ILI9488_SWRESET     0x01
#define ILI9488_RDDID       0x04
#define ILI9488_RDDST       0x09

#define ILI9488_IDMON       0x39
#define ILI9488_IDMOFF      0x38

#define ILI9488_SLPIN       0x10
#define ILI9488_SLPOUT      0x11
#define ILI9488_PTLON       0x12
#define ILI9488_NORON       0x13

#define ILI9488_RDMODE      0x0A
#define ILI9488_RDMADCTL    0x0B
#define ILI9488_RDPIXFMT    0x0C
#define ILI9488_RDIMGFMT    0x0D
#define ILI9488_RDSELFDIAG  0x0F

#define ILI9488_INVOFF      0x20
#define ILI9488_INVON       0x21
#define ILI9488_GAMMASET    0x26
#define ILI9488_DISPOFF     0x28
#define ILI9488_DISPON      0x29

#define ILI9488_CASET       0x2A
#define ILI9488_PASET       0x2B
#define ILI9488_RAMWR       0x2C
#define ILI9488_RAMRD       0x2E

#define ILI9488_PTLAR       0x30
#define ILI9488_MADCTL      0x36
#define ILI9488_PIXFMT      0x3A
#define ILI9488_RAMWRC      0x3C

#define ILI9488_FRMCTR1     0xB1
#define ILI9488_FRMCTR2     0xB2
#define ILI9488_FRMCTR3     0xB3
#define ILI9488_INVCTR      0xB4
#define ILI9488_DFUNCTR     0xB6

#define ILI9488_PWCTR1      0xC0
#define ILI9488_PWCTR2      0xC1
#define ILI9488_PWCTR3      0xC2
#define ILI9488_PWCTR4      0xC3
#define ILI9488_PWCTR5      0xC4
#define ILI9488_VMCTR1      0xC5
#define ILI9488_VMCTR2      0xC7

#define ILI9488_RDID1       0xDA
#define ILI9488_RDID2       0xDB
#define ILI9488_RDID3       0xDC
#define ILI9488_RDID4       0xDD

#define ILI9488_GMCTRP1     0xE0
#define ILI9488_GMCTRN1     0xE1

#define ILI9488_BUFFER_SIZE 64U

typedef enum {
  ILI9488_PIN_CS,
  ILI9488_PIN_DC,
  ILI9488_PIN_RST
} ILI9488Pin;

typedef enum {
  ILI9488_MODE_COMMAND,
  ILI9488_MODE_DATA
} ILI9488Mode;

typedef struct {
  int x1;
  int y1;
  int x2;
  int y2;
} ILI9488Window;

typedef struct {
  #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB111
  uint8_t                   color;
  uint8_t                   data;
  #elif LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
  uint8_t                   red;
  uint8_t                   green;
  uint8_t                   blue;
  #endif
} ILI9488Pixel;

typedef struct {
  uint8_t                   buffer1[ILI9488_BUFFER_SIZE];
  uint8_t                   buffer2[ILI9488_BUFFER_SIZE];
  uint8_t                   *active;
  size_t                    size;
} ILI9488Buffer;

typedef struct {
  Lcd                       base;
  ILI9488Mode               mode;
  ILI9488Pixel              pixel;
  ILI9488Buffer             buffer;
  ILI9488Window             window;
  void                      (*pin_set)(ILI9488Pin pin);
  void                      (*pin_reset)(ILI9488Pin pin);
  void                      (*delay_ms)(uint32_t ms);
  void                      (*spi_wait)();
  void                      (*spi_transmit)(uint8_t *tx, size_t len);
} LcdILI9488;


void ili9488_init(LcdILI9488 *lcd);
void ili9488_begin(LcdILI9488 *lcd);
void ili9488_commit(LcdILI9488 *lcd);
void ili9488_write_command(LcdILI9488 *ili9488, uint8_t data);
void ili9488_write_data(LcdILI9488 *ili9488, uint8_t data);
