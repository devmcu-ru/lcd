#include "./lcd.h"

#define LCD(x)      ((Lcd*) x)
#define SSD1306(x)  ((LcdSSD1306*) x)
#define SWAP(a, b)  { typeof(a) tmp = a; a = b; b = tmp; }


const uint8_t SSD1306_INIT[] = {
  0xAE,       // Display OFF
  0xA8, 0x3F, // Set Mux ratio to 64
  0xD3, 0x00, // Set display offset to 0
  0x20, 0x00, // Set memory addressing mode to horizontal
  0xB0,       // Set page start address to 0
  0x40,       // Set display start line to 0
  0xA1,       // Set segment remap
  0xC8,       // Set COM output scan direction to remapped
  0xDA, 0x12, // Set COM pins hardware configuration
  0x81, 0x7F, // Set Contrast Control
  0xA4,       // Disable Entire Display On
  0xA6,       // Set Normal Display
  0xD5, 0x80, // Set Osc Frequency
  0x8D, 0x14, // Enable charge pump regulator
  0xAF        // Display On
};


static void ssd1306_i2c_transmit(LcdSSD1306 *ssd1306)
{
  // No data
  SSD1306Buffer *b = &ssd1306->buffer;
  if (!b->size) return;
  // Transmit data
  ssd1306->i2c_wait();
  ssd1306->i2c_transmit(b->active, b->size);
  // Swap buffer
  b->active = b->active == b->buffer1 ? b->buffer2 : b->buffer1;
  b->size = 0;
}


void ssd1306_begin(LcdSSD1306 *ssd1306, SSD1306Stream stream)
{
  const SSD1306Stream current = ssd1306->stream;
  if (current == stream) return;
  if (stream == SSD1306_STREAM_COMMAND && current == SSD1306_STREAM_DATA) {
    ssd1306_i2c_transmit(ssd1306);
  }
  if (stream == SSD1306_STREAM_DATA) {
    if (ssd1306->buffer.size + 1 >= SSD1306_BUFFER_SIZE) {
      ssd1306_i2c_transmit(ssd1306);
    }
    ssd1306->buffer.active[ssd1306->buffer.size++] = 0x40;
  }
  ssd1306->stream = stream;
}


void ssd1306_commit(LcdSSD1306 *ssd1306)
{
  ssd1306_i2c_transmit(ssd1306);
  ssd1306->i2c_wait();
}


void ssd1306_write(LcdSSD1306 *ssd1306, uint8_t data)
{
  switch (ssd1306->stream) {
    case SSD1306_STREAM_COMMAND:
      if (ssd1306->buffer.size + 2 > SSD1306_BUFFER_SIZE) {
        ssd1306_i2c_transmit(ssd1306);
      }
      ssd1306->buffer.active[ssd1306->buffer.size++] = 0x80;
      break;
    case SSD1306_STREAM_DATA:
      if (ssd1306->buffer.size + 1 > SSD1306_BUFFER_SIZE) {
        ssd1306_i2c_transmit(ssd1306);
        ssd1306->buffer.active[ssd1306->buffer.size++] = 0x40;
      }
      break;
  }
  ssd1306->buffer.active[ssd1306->buffer.size++] = data;
}


static void ssd1306_window(LcdSSD1306 *ssd1306, uint8_t x1, uint8_t page1, uint8_t x2, uint8_t page2)
{
  ssd1306_begin(ssd1306, SSD1306_STREAM_COMMAND);
  // Set Column Address
  ssd1306_write(ssd1306, 0x21);
  ssd1306_write(ssd1306, x1);
  ssd1306_write(ssd1306, x2);
  // Set Page Address
  ssd1306_write(ssd1306, 0x22);
  ssd1306_write(ssd1306, page1);
  ssd1306_write(ssd1306, page2);
}


static void ssd1306_update_reset(LcdSSD1306 *ssd1306)
{
  const Lcd *lcd = LCD(ssd1306);
  ssd1306->update.x1 = lcd->width - 1;
  ssd1306->update.y1 = lcd->height - 1;
  ssd1306->update.x2 = 0;
  ssd1306->update.y2 = 0;
}


void ssd1306_flush(LcdSSD1306 *ssd1306)
{
  // Check update area
  const SSD1306UpdateArea *update = &ssd1306->update;
  const uint8_t x1 = update->x1;
  const uint8_t x2 = update->x2;
  if (x1 > x2 || update->y1 > update->y2) return;
  // Update area
  const uint8_t page1 = update->y1 >> 3;
  const uint8_t page2 = update->y2 >> 3;
  ssd1306_window(ssd1306, x1, page1, x2, page2);
  // Send frame to LCD
  const Lcd *lcd = LCD(ssd1306);
  ssd1306_begin(ssd1306, SSD1306_STREAM_DATA);
  for (uint8_t page = page1; page <= page2; page ++) {
    const uint8_t *buf = ssd1306->frame + (size_t) page * lcd->width + x1;
    for (uint8_t x = x1; x <= x2; x ++) {
      ssd1306_write(ssd1306, *buf ++);
    }
  }
  ssd1306_commit(ssd1306);
  // Reset update area
  ssd1306_update_reset(ssd1306);
}


static void ssd1306_color(const Lcd *lcd, Color color)
{
  SSD1306(lcd)->pixel = color == COLOR_BLACK
    ? SSD1306_PIXEL_BLACK
    : SSD1306_PIXEL_WHITE;
}


static void ssd1306_pixel(const Lcd *lcd, int x, int y)
{
  // Out of display
  if ((unsigned) x >= lcd->width || ((unsigned) y >= lcd->height)) return;
  // Update frame buffer
  LcdSSD1306 *ssd1306 = SSD1306(lcd);
  const uint8_t col = (uint8_t) x;
  const uint8_t row = (uint8_t) y;
  const uint8_t page = row >> 3;
  uint8_t *b = ssd1306->frame + (size_t) page * lcd->width + col;
  const uint8_t old = *b;
  const uint8_t mask = (uint8_t) (1u << (row & 7));
  if (ssd1306->pixel == SSD1306_PIXEL_WHITE) *b |= mask;
  else *b &= (uint8_t) ~mask;
  if (old == *b) return;
  // Update area
  SSD1306UpdateArea *update = &ssd1306->update;
  if (col < update->x1) update->x1 = col;
  if (col > update->x2) update->x2 = col;
  if (row < update->y1) update->y1 = row;
  if (row > update->y2) update->y2 = row;
}


static void ssd1306_fill(const Lcd *lcd, int x1, int y1, int x2, int y2)
{
  // Normalize coordinates
  if (x1 > x2) SWAP(x1, x2);
  if (y1 > y2) SWAP(y1, y2);
  // Out of display
  if (x2 < 0 || x1 >= lcd->width) return;
  if (y2 < 0 || y1 >= lcd->height) return;
  // Clip viewport
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 >= lcd->width) x2 = lcd->width - 1;
  if (y2 >= lcd->height) y2 = lcd->height - 1;
  // Pages
  const uint8_t page1 = y1 >> 3;
  const uint8_t page2 = y2 >> 3;
  // Mask
  const uint8_t bit1 = y1 & 7;
  const uint8_t bit2 = y2 & 7;
  const uint8_t mask1 = 0xFF << bit1;
  const uint8_t mask2 = 0xFF >> (7 - bit2);
  // Fill
  LcdSSD1306 *ssd1306 = SSD1306(lcd);
  uint8_t *frame = ssd1306->frame;
  const uint8_t white = ssd1306->pixel == SSD1306_PIXEL_WHITE;
  uint8_t update_x1 = (uint8_t) x2;
  uint8_t update_x2 = (uint8_t) x1;
  for (uint8_t page = page1; page <= page2; page++) {
    uint8_t mask;
    if (page1 == page2) mask = mask1 & mask2;
    else if (page == page1) mask = mask1;
    else if (page == page2) mask = mask2;
    else mask = 0xFF;
    // Fill columns
    uint8_t *buf = frame + (size_t) page * lcd->width + x1;
    for (int x = x1; x <= x2; x ++) {
      const uint8_t old = *buf;
      uint8_t b = old;
      if (white) b |= mask;
      else b &= (uint8_t) ~mask;
      if (b != old) {
        *buf = b;
        if (x < update_x1) update_x1 = (uint8_t) x;
        if (x > update_x2) update_x2 = (uint8_t) x;
      }
      buf ++;
    }
  }
  if (update_x1 > update_x2) return;
  // Update area
  SSD1306UpdateArea *update = &ssd1306->update;
  if (x1 < update->x1) update->x1 = x1;
  if (x2 > update->x2) update->x2 = x2;
  if (y1 < update->y1) update->y1 = y1;
  if (y2 > update->y2) update->y2 = y2;
}


static void ssd1306_clear(const Lcd *lcd)
{
  ssd1306_fill(lcd, 0, 0, lcd->width - 1, lcd->height - 1);
}

const LcdDriver ssd1306_driver = {
  .color = ssd1306_color,
  .pixel = ssd1306_pixel,
  .fill = ssd1306_fill,
  .clear = ssd1306_clear
};


void ssd1306_init(LcdSSD1306 *ssd1306)
{
  // Driver config
  ssd1306->base.driver = ssd1306_driver;
  ssd1306->stream = SSD1306_STREAM_IDLE;
  // Reset buffer
  ssd1306->buffer.active = ssd1306->buffer.buffer1;
  ssd1306->buffer.size = 0;
  // Invalidate update area
  ssd1306->update.x1 = 0;
  ssd1306->update.y1 = 0;
  ssd1306->update.x2 = ssd1306->base.width - 1;
  ssd1306->update.y2 = ssd1306->base.height - 1;
  // Initialize
  ssd1306_begin(ssd1306, SSD1306_STREAM_COMMAND);
  for (size_t n = 0; n < sizeof(SSD1306_INIT); n++) {
    ssd1306_write(ssd1306, SSD1306_INIT[n]);
  }
  ssd1306_commit(ssd1306);
}
