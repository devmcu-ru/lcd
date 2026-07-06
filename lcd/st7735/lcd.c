#include "./lcd.h"


#define SWAP(a, b)            { typeof(a) tmp = a; a = b; b = tmp; }

#if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB565
  #define CHUNK_SIZE          ((ST7735_BUFFER_SIZE) / 2U)
  #define CHUNK_STEP          1U
  #define BYTES_PER_PIXEL     2U
#elif LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
  #define CHUNK_SIZE          ((ST7735_BUFFER_SIZE) / 3U)
  #define CHUNK_STEP          1U
  #define BYTES_PER_PIXEL     3U
#else
  #error ST7735 unknown pixel format
#endif

#define CHUNK_PIXELS          (ST7735_BUFFER_SIZE / BYTES_PER_PIXEL)
#define CHUNK_BYTES           (CHUNK_PIXELS * BYTES_PER_PIXEL)

#define LCD_MADCTL_LANDSCAPE  0b01100000
#define LCD_MADCTL_PORTRAIT   0b00000000
#define LCD(lcd)              ((LcdST7735 *) lcd)


static void st7735_spi_transmit(const Lcd *lcd)
{
  // No data
  ST7735Buffer *b = &LCD(lcd)->buffer;
  if (!b->size) return;

  // Transmit data
  LCD(lcd)->spi_wait();
  LCD(lcd)->spi_transmit(b->active, b->size);

  // Swap buffer
  b->active = b->active == b->buffer1 ? b->buffer2 : b->buffer1;
  b->size = 0;
}


static void st7735_spi_write(const Lcd *lcd, uint8_t data)
{
  ST7735Buffer *b = &LCD(lcd)->buffer;
  b->active[b->size ++] = data;
  if (b->size >= ST7735_BUFFER_SIZE) st7735_spi_transmit(lcd);
}


static inline void st7735_begin(const Lcd *lcd)
{
  LCD(lcd)->pin_reset(ST7735_PIN_CS);
}


static void st7735_commit(const Lcd *lcd)
{
  st7735_spi_transmit(lcd);
  LCD(lcd)->spi_wait();
  LCD(lcd)->pin_set(ST7735_PIN_CS);
}


static void st7735_switch_data(const Lcd *lcd)
{
  if (LCD(lcd)->mode != ST7735_MODE_DATA) {
    st7735_spi_transmit(lcd);
    LCD(lcd)->spi_wait();
    LCD(lcd)->pin_set(ST7735_PIN_DC);
    LCD(lcd)->mode = ST7735_MODE_DATA;
  }
}


static void st7735_switch_command(const Lcd *lcd)
{
  if (LCD(lcd)->mode != ST7735_MODE_COMMAND) {
    st7735_spi_transmit(lcd);
    LCD(lcd)->spi_wait();
    LCD(lcd)->pin_reset(ST7735_PIN_DC);
    LCD(lcd)->mode = ST7735_MODE_COMMAND;
  }
}


static void st7735_write_data(const Lcd *lcd, uint8_t data)
{
  st7735_switch_data(lcd);
  st7735_spi_write(lcd, data);
}


static void st7735_write_command(const Lcd *lcd, uint8_t data)
{
  st7735_switch_command(lcd);
  st7735_spi_write(lcd, data);
}


static void st7735_window(const Lcd *lcd, int x1, int y1, int x2, int y2)
{
  // Изменилось окно
  ST7735Window *window = &LCD(lcd)->window;
  const bool changed_caset = (window->x1 != x1) || (window->x2 != x2);
  const bool changed_paset = (window->y1 != y1) || (window->y2 != y2);

  // Column address
  if (changed_caset) {
    st7735_write_command(lcd, ST7735_CASET);
    st7735_write_data(lcd, x1 >> 8);
    st7735_write_data(lcd, x1 & 0xFF);
    st7735_write_data(lcd, x2 >> 8);
    st7735_write_data(lcd, x2 & 0xFF);
    window->x1 = x1;
    window->x2 = x2;
  }

  // Page address
  if (changed_paset) {
    st7735_write_command(lcd, ST7735_PASET);
    st7735_write_data(lcd, y1 >> 8);
    st7735_write_data(lcd, y1 & 0xFF);
    st7735_write_data(lcd, y2 >> 8);
    st7735_write_data(lcd, y2 & 0xFF);
    window->y1 = y1;
    window->y2 = y2;
  }

  // Memory write
  st7735_write_command(lcd, ST7735_RAMWR);
  st7735_switch_data(lcd);
}


static void st7735_color(const Lcd *lcd, Color color)
{
  #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
  LCD(lcd)->pixel.blue = color & 0xFC;
  LCD(lcd)->pixel.green = (color >> 8) & 0xFC;
  LCD(lcd)->pixel.red = (color >> 16) & 0xFC;
  #elif LCD_PIXEL_FORMAT == LCD_PIXEL_RGB565
  const uint8_t blue = (color >> 3) & 0x1F;
  const uint8_t green = (color >> 10) & 0x3F;
  const uint8_t red = (color >> 19) & 0x1F;
  LCD(lcd)->pixel.data[0] = (red << 3) | (green >> 3);
  LCD(lcd)->pixel.data[1] = (green << 5) | blue;
  #endif
}


static void st7735_pixel(const Lcd *lcd, int x, int y)
{
  // Visibility
  if (x < 0 || x >= lcd->width) return;
  if (y < 0 || y >= lcd->height) return;

  // Start transaction
  st7735_begin(lcd);
  st7735_window(lcd, x, y, x, y);

  // Draw pixel
  #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB565
  st7735_write_data(lcd, LCD(lcd)->pixel.data[0]);
  st7735_write_data(lcd, LCD(lcd)->pixel.data[1]);
  #elif LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
  st7735_write_data(lcd, LCD(lcd)->pixel.red);
  st7735_write_data(lcd, LCD(lcd)->pixel.green);
  st7735_write_data(lcd, LCD(lcd)->pixel.blue);
  #endif

  // Commit transaction
  st7735_commit(lcd);
}


static void st7735_fill(const Lcd *lcd, int x1, int y1, int x2, int y2)
{
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
    st7735_pixel(lcd, ax, ay);
    return;
  }

  // Pixels
  const size_t chunk_pixels = (size < CHUNK_PIXELS) ? size : CHUNK_PIXELS;
  if (!chunk_pixels) return;

  // Start transaction
  st7735_begin(lcd);
  st7735_window(lcd, ax, ay, bx, by);

  // Prepare chunk
  const ST7735Pixel pixel = LCD(lcd)->pixel;
  uint8_t *const buffer = LCD(lcd)->buffer.active;
  uint8_t *chunk = buffer;
  for (size_t n = 0; n < chunk_pixels; n += CHUNK_STEP) {
    #if LCD_PIXEL_FORMAT == LCD_PIXEL_RGB565
    *(chunk ++) = pixel.data[0];
    *(chunk ++) = pixel.data[1];
    #elif LCD_PIXEL_FORMAT == LCD_PIXEL_RGB666
    *(chunk ++) = pixel.red;
    *(chunk ++) = pixel.green;
    *(chunk ++) = pixel.blue;
    #endif
  }

  // Chunk size in bytes
  const size_t chunk_bytes = chunk_pixels * BYTES_PER_PIXEL;

  // Fill by chunks
  size_t sent_pixels = 0;
  while (sent_pixels + chunk_pixels <= size) {
    LCD(lcd)->spi_wait();
    LCD(lcd)->spi_transmit(buffer, chunk_bytes);
    sent_pixels += chunk_pixels;
  }

  // Transmit the remaining chunk
  if (sent_pixels < size) {
    const size_t remain_bytes = (size - sent_pixels) * BYTES_PER_PIXEL;
    LCD(lcd)->spi_wait();
    LCD(lcd)->spi_transmit(buffer, remain_bytes);
  }

  // Commit transaction
  st7735_commit(lcd);
}


static void st7735_clear(const Lcd *lcd)
{
  st7735_fill(lcd, 0, 0, lcd->width - 1, lcd->height - 1);
}


const LcdDriver st7735_driver = {
  .color = st7735_color,
  .pixel = st7735_pixel,
  .fill  = st7735_fill,
  .clear = st7735_clear
};


void st7735_init(LcdST7735 *lcd)
{
  // Driver config
  lcd->base.width = LCD_WIDTH;
  lcd->base.height = LCD_HEIGHT;
  lcd->base.driver = st7735_driver;

  // Reset buffer
  lcd->buffer.active = lcd->buffer.buffer1;
  lcd->buffer.size = 0;

  // Window
  lcd->window.x1 = -1;
  lcd->window.y1 = -1;
  lcd->window.x2 = -1;
  lcd->window.y2 = -1;

  // Data / command mode
  LCD(lcd)->mode = ST7735_MODE_DATA;

  // Hardware reset
  LCD(lcd)->pin_reset(ST7735_PIN_RST);
  LCD(lcd)->delay_ms(50);
  LCD(lcd)->pin_set(ST7735_PIN_RST);
  LCD(lcd)->delay_ms(150);

  // Start transfer
  Lcd *base = (Lcd *) lcd;
  st7735_begin(base);

  // Sleep OUT
  st7735_write_command(base, ST7735_SLPOUT);
  LCD(lcd)->spi_wait();
  LCD(lcd)->delay_ms(120);

  // Display ON
  st7735_write_command(base, ST7735_DISPON);

  // Interface Pixel Format
  st7735_write_command(base, ST7735_PIXFMT);
  st7735_write_data(base, LCD_PIXEL_FORMAT);

  // Memory Access Control
  st7735_write_command(base, ST7735_MADCTL);
  st7735_write_data(base, LCD_LANDSCAPE ? LCD_MADCTL_LANDSCAPE : LCD_MADCTL_PORTRAIT);
  
  // Finish transfer
  st7735_commit(base);
}