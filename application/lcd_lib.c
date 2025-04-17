#include "lcd_lib.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

static uint16_t frame_buffer[LCD_WIDTH * LCD_HEIGHT];

int lcd_write(){
    int fd = open(LCD_DEV_FILE, O_WRONLY);
    if (fd < 0) {
        printf("lcd_lib Error: Cannot open device file\n");
        return -1;
    }

    int ret = write(fd, frame_buffer, sizeof(frame_buffer));
    if (!ret) {
        printf("lcd_lib Error: Cannot write to device file\n");
        return ret;
    }
    close(fd);
    return 0;
}

int draw_filled_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color){
    if ((x+r > LCD_WIDTH) || (y+r > LCD_HEIGHT)) {
        printf("lcd_lib Error: drawing out of screen bounds\n");
        return -1;
	}
    for (uint16_t i=y-r; i<=y+r; i++) {
        for (uint16_t j=x-r; j<=x+r; j++) {
            if ((y-i)*(y-i) + (x-j)*(x-j) <= r*r+1) {
                frame_buffer[i + LCD_WIDTH * j] = (color >> 8) | (color << 8);
            }
        }
    }
    return 0;
}


int draw_filled_rect(uint16_t x,  uint16_t y, uint16_t w, uint16_t h, uint16_t color){
    if ((x+w > LCD_WIDTH) || (y+h > LCD_HEIGHT)) {
        printf("lcd_lib Error: drawing out of screen bounds\n");
        return -1;
	}
    for (uint32_t j = 0; j < h; j++) {
		for (uint32_t i = 0; i < w; i++) {
			frame_buffer[(x + LCD_WIDTH * y) + (i + LCD_WIDTH * j)] = (color >> 8) | (color << 8);
		}
	}
    return 0;
}

int draw_full_screen(uint16_t* buffer){
    memcpy(frame_buffer, buffer, sizeof(frame_buffer));
    return 0;
}

int draw_char(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor){
    if ((x+font.width >= LCD_WIDTH) || (y+font.height >= LCD_HEIGHT)) {
        printf("lcd_lib Error: drawing out of screen bounds\n");
        return -1;
	}
    uint32_t i, b, j;

	for (i = 0; i < font.height; i++) {
		b = font.data[(ch - 32) * font.height + i];
		for (j = 0; j < font.width; j++) {
			if ((b << j) & 0x8000)  {
				frame_buffer[(x + LCD_WIDTH * y) + (j + LCD_WIDTH * i)] = 
					(color >> 8) | (color << 8);
			} 
			else {
				frame_buffer[(x + LCD_WIDTH * y) + (j + LCD_WIDTH * i)] = 
					(bgcolor >> 8) | (bgcolor << 8);
			}
		}
	}
    return 0;
}

int draw_str(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor){
    while (*str) {
        if (x + font.width >= LCD_WIDTH) {
            x = 0;
            y += font.height;
            if (y + font.height >= LCD_HEIGHT) {
                printf("lcd_lib Error: drawing out of screen bounds\n");
                return -1;
            }

            if (*str == ' ') {
                str++;
                continue;
            }
        }
        draw_char(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
    return 0;
}
