#include <stdint.h>
#include "fonts.h"

#define LCD_DEV_FILE "/dev/lcd_ili9341"
#define LCD_WIDTH  320
#define LCD_HEIGHT 240

// Color definitions
#define COLOR_BLACK   0x0000
#define COLOR_BLUE    0x001F
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_YELLOW  0xFFE0
#define COLOR_WHITE   0xFFFF
#define COLOR_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))


int lcd_write(void);
int draw_filled_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color);
int draw_filled_rect(uint16_t x,  uint16_t y, uint16_t w, uint16_t h, uint16_t color);
int draw_full_screen(uint16_t* buffer);
int draw_char(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
int draw_str(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);