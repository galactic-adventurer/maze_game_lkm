#ifndef __ILI9341_H__
#define __ILI9341_H__

#define LCD_PIN_CS 7
#define LCD_PIN_RESET 27
#define LCD_PIN_DC 22

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04

// default orientation
#define LCD_WIDTH  320
#define LCD_HEIGHT 240
#define LCD_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR)

#define LCD_CASET 0x2A
#define LCD_RASET 0x2B
#define LCD_RAMWR 0x2C

#endif // __ILI9341_H__