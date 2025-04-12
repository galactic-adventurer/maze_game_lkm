#include <stdio.h>
#include <stdlib.h>
#include "lcd_lib.h"
#include "fonts.c"

int main() {

    draw_str(12, 24, "Hello World!", Font_11x18, COLOR_CYAN, COLOR_BLACK);
    draw_filled_circle(100, 100, 20, COLOR_WHITE);
    
    return 0;
}
