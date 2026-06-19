#include "./lcd.h"


#if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB111
  #define CHUNK_SIZE          ((ILI9488_BUFFER_SIZE) * 2U)
  #define CHUNK_STEP          2
#elif LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
  #define CHUNK_SIZE          ((ILI9488_BUFFER_SIZE) / 3U)
  #define CHUNK_STEP          1
#endif

#define LCD_MADCTL_LANDSCAPE  0b00101000
#define LCD_MADCTL_PORTRAIT   0b00001000
#define ILI9488(lcd)		  ((LcdILI9488 *) lcd)


static void ili9488_spi_transmit(LcdILI9488 *ili9488)
{
  // No data
  ILI9488Buffer *b = &ili9488->buffer;
  if (!b->size) return;

  // Transmit data
  ili9488->spi_wait();
  ili9488->spi_transmit(b->active, b->size);

  // Swap buffer
  b->active = b->active == b->buffer1 ? b->buffer2 : b->buffer1;
  b->size = 0;
}


static void ili9488_spi_write(LcdILI9488 *ili9488, uint8_t data)
{
  ILI9488Buffer *b = &ili9488->buffer;
  b->active[b->size ++] = data;
  if (b->size >= ILI9488_BUFFER_SIZE) ili9488_spi_transmit(ili9488);
}


void ili9488_begin(LcdILI9488 *ili9488)
{
  ili9488->pin_reset(ILI9488_PIN_CS);
}


void ili9488_commit(LcdILI9488 *ili9488)
{
  ili9488_spi_transmit(ili9488);
  ili9488->spi_wait();
  ili9488->pin_set(ILI9488_PIN_CS);
}


static void ili9488_switch_data(LcdILI9488 *ili9488)
{
  if (ili9488->mode != ILI9488_MODE_DATA) {
    ili9488_spi_transmit(ili9488);
    ili9488->spi_wait();
    ili9488->pin_set(ILI9488_PIN_DC);
    ili9488->mode = ILI9488_MODE_DATA;
  }
}


static void ili9488_switch_command(LcdILI9488 *ili9488)
{
  if (ili9488->mode != ILI9488_MODE_COMMAND) {
    ili9488_spi_transmit(ili9488);
    ili9488->spi_wait();
    ili9488->pin_reset(ILI9488_PIN_DC);
    ili9488->mode = ILI9488_MODE_COMMAND;
  }
}


void ili9488_write_data(LcdILI9488 *ili9488, uint8_t data)
{
  ili9488_switch_data(ili9488);
  ili9488_spi_write(ili9488, data);
}


void ili9488_write_command(LcdILI9488 *ili9488, uint8_t data)
{
  ili9488_switch_command(ili9488);
  ili9488_spi_write(ili9488, data);
}


static void ili9488_window(LcdILI9488 *ili9488, int x1, int y1, int x2, int y2)
{
  // Изменилось окно
  ILI9488Window *window = &ili9488->window;
  const bool changed_caset = (window->x1 != x1) || (window->x2 != x2);
  const bool changed_paset = (window->y1 != y1) || (window->y2 != y2);

  // В режиме RGB111 при выборе окна
  // экран глючит, но если выбрать RGB666 работает
  #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB111
  if (changed_paset || changed_caset) {
    ili9488_write_command(ili9488, ILI9488_PIXFMT);
    ili9488_write_data(ili9488, LCD_PIXEL_RGB666);
  }
  #endif

  // Column address
  if (changed_caset) {
    ili9488_write_command(ili9488, ILI9488_CASET);
    ili9488_write_data(ili9488, x1 >> 8);
    ili9488_write_data(ili9488, x1 & 0xFF);
    ili9488_write_data(ili9488, x2 >> 8);
    ili9488_write_data(ili9488, x2 & 0xFF);
    window->x1 = x1;
    window->x2 = x2;
  }

  // Page address
  if (changed_paset) {
    ili9488_write_command(ili9488, ILI9488_PASET);
    ili9488_write_data(ili9488, y1 >> 8);
    ili9488_write_data(ili9488, y1 & 0xFF);
    ili9488_write_data(ili9488, y2 >> 8);
    ili9488_write_data(ili9488, y2 & 0xFF);
    window->y1 = y1;
    window->y2 = y2;
  }

  // Переключаемся обратно в RGB111
  #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB111
  if (changed_paset || changed_caset) {
    ili9488_write_command(ili9488, ILI9488_PIXFMT);
    ili9488_write_data(ili9488, LCD_PIXEL_RGB111);
  }
  #endif

  // Memory write
  ili9488_write_command(ili9488, ILI9488_RAMWR);
  ili9488_switch_data(ili9488);
}


static void ili9488_color(const Lcd *lcd, Color color)
{
  LcdILI9488 *const ili9488 = ILI9488(lcd);

  // RGB666 Pixel Format
  #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
  ili9488->pixel.blue = color & 0xFF;
  ili9488->pixel.green = (color >> 8) & 0xFF;
  ili9488->pixel.red = (color >> 16) & 0xFF;
  // RGB111 Pixel Format
  #elif LCD_PIXEL_FORMAT == LCD_PIXEL_RGB111
  uint8_t rgb111 = \
    ((color >> 21) & 0x04) \
    | ((color >> 14) & 0x02) \
    | ((color >> 7) & 0x01);
  ili9488->pixel.color = rgb111;
  ili9488->pixel.data = (rgb111 << 3) | rgb111;
  #endif
}


static void ili9488_pixel(const Lcd *lcd, int x, int y)
{
  LcdILI9488 *const ili9488 = ILI9488(lcd);

  // Visibility
  if (x < 0 || x >= lcd->width) return;
  if (y < 0 || y >= lcd->height) return;

  // Start transaction
  ili9488_begin(ili9488);
  ili9488_window(ili9488, x, y, x, y);

  // Draw pixel
  #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB111
  ili9488_write_data(ili9488, ili9488->pixel.data);
  #elif LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
  ili9488_write_data(ili9488, ili9488->pixel.red);
  ili9488_write_data(ili9488, ili9488->pixel.green);
  ili9488_write_data(ili9488, ili9488->pixel.blue);
  #endif

  // Commit transaction
  ili9488_commit(ili9488);
}


static void ili9488_fill(const Lcd *lcd, int x1, int y1, int x2, int y2)
{
  LcdILI9488 *const ili9488= ILI9488(lcd);

  // Max window size
  const int max_x = lcd->width - 1;
  const int max_y = lcd->height - 1;

  // Window X coordinates
  int ax, bx;
  if (x1 > x2) { ax = x2; bx = x1; }
  else { ax = x1; bx = x2; }
  if (ax < 0) ax = 0;
  else if (ax > max_x) ax = max_x;
  if (bx < 0) bx = 0;
  else if (bx > max_x) bx = max_x;

  // Window Y coordinates
  int ay, by;
  if (y1 > y2) { ay = y2; by = y1; }
  else { ay = y1; by = y2; }
  if (ay < 0) ay = 0;
  else if (ay > max_y) ay = max_y;
  if (by < 0) by = 0;
  else if (by > max_y) by = max_y;

  // 1-pixel window — draw a single pixel
  const size_t width = (size_t) (bx - ax + 1);
  const size_t height = (size_t) (by - ay + 1);
  const size_t size = width * height;
  if (size == 1) {
    ili9488_pixel(lcd, ax, ay);
    return;
  }

  // Start transaction
  ili9488_begin(ili9488);
  ili9488_window(ili9488, ax, ay, bx, by);

  // Prepare chunk
  const ILI9488Pixel pixel = ili9488->pixel;
  uint8_t *const buffer = ili9488->buffer.active;
  uint8_t *chunk = buffer;
  size_t chunk_size = 0;
  while (chunk_size < size && chunk_size < CHUNK_SIZE) {
    #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB111
    *(chunk ++) = pixel.data;
    #elif LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
    *(chunk ++) = pixel.red;
    *(chunk ++) = pixel.green;
    *(chunk ++) = pixel.blue;
    #endif
    chunk_size += CHUNK_STEP;
  }

  // Chunk size in bytes
  #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
  const size_t chunk_bytes = chunk_size * 3;
  #else
  const size_t chunk_bytes = (chunk_size + 1) / 2;
  #endif

  // Fill by chunks
  size_t n = 0;
  while (n + chunk_size <= size) {
	ili9488->spi_wait();
	ili9488->spi_transmit(buffer, chunk_bytes);
    n += chunk_size;
  }

  // Transmit the remaining chunk
  if (n < size) {
    #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
    const size_t chunk_remain = (size - n) * 3;
    #else
    const size_t chunk_remain = (size - n + 1) / 2;
    #endif
    ili9488->spi_wait();
    ili9488->spi_transmit(buffer, chunk_remain);
  }

  // Commit transaction
  ili9488_commit(ili9488);
}


static void ili9488_clear(const Lcd *lcd)
{
  ili9488_fill(lcd, 0, 0, lcd->width - 1, lcd->height - 1);
}


const LcdDriver ili9488_driver = {
  .color = ili9488_color,
  .pixel = ili9488_pixel,
  .fill  = ili9488_fill,
  .clear = ili9488_clear
};


void ili9488_init(LcdILI9488 *il9488)
{
  // Driver config
  il9488->base.width = LCD_WIDTH;
  il9488->base.height = LCD_HEIGHT;
  il9488->base.driver = ili9488_driver;

  // Reset buffer
  il9488->buffer.active = il9488->buffer.buffer1;
  il9488->buffer.size = 0;

  // Window
  il9488->window.x1 = -1;
  il9488->window.y1 = -1;
  il9488->window.x2 = -1;
  il9488->window.y2 = -1;

  // Reset sequence
  il9488->pin_set(ILI9488_PIN_RST);
  il9488->pin_set(ILI9488_PIN_CS);
  il9488->pin_set(ILI9488_PIN_DC);
  il9488->delay_ms(100);
  il9488->pin_reset(ILI9488_PIN_RST);
  il9488->delay_ms(500);
  il9488->pin_set(ILI9488_PIN_RST);
  il9488->delay_ms(500);

  // Data / command mode
  il9488->mode = ILI9488_MODE_DATA;

  // Start transfer
  ili9488_begin(il9488);

  // Sleep OUT
  ili9488_write_command(il9488, ILI9488_SLPOUT);
  il9488->spi_wait();
  il9488->delay_ms(120);

  // Display ON
  ili9488_write_command(il9488, ILI9488_DISPON);

  // Interface Pixel Format
  ili9488_write_command(il9488, ILI9488_PIXFMT);
  ili9488_write_data(il9488, LCD_PIXEL_FORMAT);

  // Memory Access Control
  ili9488_write_command(il9488, ILI9488_MADCTL);
  ili9488_write_data(il9488, LCD_LANDSCAPE ? LCD_MADCTL_LANDSCAPE : LCD_MADCTL_PORTRAIT);

  // Finish transfer
  ili9488_commit(il9488);
}
