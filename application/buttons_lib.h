#include <sys/ioctl.h>

#define BUTTONS_DEV_FILE "/dev/lcd_button_char_dev"


int buttons_open();
int get_button_pressed();
void buttons_close();

