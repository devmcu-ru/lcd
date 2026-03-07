# Общие сведения

Библиотека машинной графики для микроконтроллеров, размер ОЗУ которых не позволяет полноценно работать с фреймбуфером.
Библиотека не привязана к какой-либо архитектуре микроконтроллера, вместо этого драйверы используются колбеки -
функции который реализует пользователь и передает при инициализации драйвера.

Такой подход позволяет максимально использовать периферию (SPI, DMA, ...) микроконтроллеров и при этом код остается переносимым

## Возможности

Библиотека разбита на два слоя:
* Слой драйверов - позволяет использовать поддерживаемые экраны
* Слой графики - базовые примитивы рисования фигур / шрифтов

### Слой драйверов

На текущий момент поддерживаются экраны:
* ILI9488 (4-wire SPI, RGB111, RGB666)
* ST7735 (4-wire SPI, RGB565, RGB666)

Пример инициализации драйвера ST7735 при использовании STM32 + DMA
```c
#include "lcd/st7735/lcd.h"
#include "main.h"
#include "app/lcd.h"
#include "app/delay.h"

#define dma_tc_active(v)  LL_DMA_IsActiveFlag_TC1(v)
#define dma_tc_clear(v)   LL_DMA_ClearFlag_TC1(v)
#define dma_te_active(v)  LL_DMA_IsActiveFlag_TE1(v)
#define dma_te_clear(v)   LL_DMA_ClearFlag_TE1(v)
#define dma_gi_clear(v)   LL_DMA_ClearFlag_GI1(v)


void lcd_spi_wait()
{
  // Если DMA активен, ждем завершения передачи
  if (LL_DMA_IsEnabledChannel(LCD_DMA, LCD_DMA_CHANNEL)) {
    while (!dma_tc_active(LCD_DMA) && !dma_te_active(LCD_DMA)) { }
    dma_tc_clear(LCD_DMA);
    dma_te_clear(LCD_DMA);
    LL_DMA_DisableChannel(LCD_DMA, LCD_DMA_CHANNEL);
  }

  // Ждем завершения передачи SPI
  while (LL_SPI_IsActiveFlag_BSY(LCD_SPI));
}


void lcd_spi_transmit(uint8_t *tx, size_t size)
{
  // Слишком мало данных, нет смысла использовать DMA
  // передаем данные в блокирующем режиме
  if (size <= 4) {
    while (size --) {
      LL_SPI_TransmitData8(LCD_SPI, *(tx ++));
      if (size > 0) while (!LL_SPI_IsActiveFlag_TXE(LCD_SPI));
    }
    return;
  }

  // Останавливаем передачу
  LL_DMA_DisableChannel(LCD_DMA, LCD_DMA_CHANNEL);
  dma_gi_clear(LCD_DMA);

  // Настраиваем буфер передачи
  LL_DMA_SetPeriphAddress(LCD_DMA, LCD_DMA_CHANNEL, (uint32_t) &LCD_SPI->DR);
  LL_DMA_SetMemoryAddress(LCD_DMA, LCD_DMA_CHANNEL, (uint32_t) tx);
  LL_DMA_SetDataLength(LCD_DMA, LCD_DMA_CHANNEL, size);

  // Запускаем передачу
  LL_DMA_EnableChannel(LCD_DMA, LCD_DMA_CHANNEL);
}

void st7735_pin_set(ST7735Pin pin)
{
  switch (pin) {
    case ST7735_PIN_CS:
      LL_GPIO_SetOutputPin(LCD_CS_GPIO_Port, LCD_CS_Pin);
      break;
    case ST7735_PIN_DC:
      LL_GPIO_SetOutputPin(LCD_DC_GPIO_Port, LCD_DC_Pin);
      break;
    case ST7735_PIN_RST:
      LL_GPIO_SetOutputPin(LCD_RST_GPIO_Port, LCD_RST_Pin);
      break;
  }
}


void st7735_pin_reset(ST7735Pin pin)
{
  switch (pin) {
    case ST7735_PIN_CS:
      LL_GPIO_ResetOutputPin(LCD_CS_GPIO_Port, LCD_CS_Pin);
      break;
    case ST7735_PIN_DC:
      LL_GPIO_ResetOutputPin(LCD_DC_GPIO_Port, LCD_DC_Pin);
      break;
    case ST7735_PIN_RST:
      LL_GPIO_ResetOutputPin(LCD_RST_GPIO_Port, LCD_RST_Pin);
      break;
  }
}

static LcdST7735 lcd = {
  .pin_set = st7735_pin_set,
  .pin_reset = st7735_pin_reset,
  .spi_wait = lcd_spi_wait,
  .spi_transmit = lcd_spi_transmit,
  .delay_ms = delay_ms
};

Lcd *lcd_init()
{
  st7735_init(&lcd);
  return &lcd.base;
}
```

### Слой графики

Слой графики (Canvas) позволяет выводить базовые примитивы на экран:
```c
// Инициализация
void canvas_init(Canvas *canvas, const Lcd *lcd);

// Полная очистка экрана
void canvas_clear(Canvas *canvas);

// Выбор цвета заливки
void canvas_color(Canvas *canvas, Color color); 

// Выбор шрифта
void canvas_font(Canvas *canvas, const LCDFont *font);

// Задать область исключения для рисования
void canvas_clip(Canvas *canvas, const CanvasRectangle *rect);

// Нарисовать пиксель
void canvas_pixel(Canvas *canvas, int x, int y);

// Заполнить прямоугольную область
void canvas_fill(Canvas *canvas, int x1, int y1, int x2, int y2);`

// Нарисовать линию
void canvas_line(Canvas *canvas, int x1, int y1, int x2, int y2);`

// Нарисовать прямоугольник
void canvas_rectangle(Canvas *canvas, int x1, int y1, int x2, int y2);`

// Нарисовать окружность
void canvas_circle(Canvas* canvas, int xc, int yx, int r);`

// Нарисовать крестик
void canvas_cross(Canvas *canvas, int x, int y, uint8_t size);`

// Нарисовать символ, возвращается ширина символа
size_t canvas_char(Canvas *canvas, int x, int y, char c);`

// Нарисовать строку
void canvas_text(Canvas *canvas, int x, int y, const char *text);````
