#define LCD_DEV_FILE "/dev/lcd_display"
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
int draw_filled_circle(u16 x, u16 y, u16 r, u16 color);
int draw_filled_rect(u16 x,  u16 y, u16 w, u16 h, u16 color);
int draw_full_screen(u16* buffer);
int draw_char(u16 x, u16 y, char ch, FontDef font, u16 color, u16 bgcolor);
int draw_str(u16 x, u16 y, const char* str, FontDef font, u16 color, u16 bgcolor);