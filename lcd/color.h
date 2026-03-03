#ifndef __LCD_COLOR_H_
#define __LCD_COLOR_H_


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#define COLOR_RGB(r, g, b)  (((uint32_t) r << 16) | ((uint32_t) g << 8) | b)
#define COLOR_BLACK         COLOR_RGB(0, 0, 0)
#define COLOR_RED           COLOR_RGB(255, 0, 0)
#define COLOR_GREEN         COLOR_RGB(0, 255, 0)
#define COLOR_YELLOW        COLOR_RGB(255, 255, 0)
#define COLOR_BLUE          COLOR_RGB(0, 0, 255)
#define COLOR_MAGENTA       COLOR_RGB(255, 0, 255)
#define COLOR_CYAN          COLOR_RGB(0, 255, 255)
#define COLOR_WHITE         COLOR_RGB(255, 255, 255)


typedef uint32_t Color;


#ifdef __cplusplus
}
#endif


#endif /* __LCD_COLOR_H_ */
