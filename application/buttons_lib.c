#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "buttons_lib.h"

static int fd;
char buffer[2];

int buttons_open(){
    fd = open(BUTTONS_DEV_FILE, O_RDONLY);
    if (fd < 0) {
      printf("Cannot open mpu device file...\n");
      return -1;
    }
    return 0;
}

int get_button_pressed(){
    read(fd, buffer, sizeof(buffer)); // Blocks until interrupt occurs
    printf("Button %s pressed!\n", buffer);
    return buffer[0] - 48; //convert from ASCII to int
}

void buttons_close(){
    close(fd);
}