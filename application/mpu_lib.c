#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "mpu_lib.h"

static int fd;

int mpu_open(){
    fd = open(MPU_DEV_FILE, O_RDONLY);
    if (fd < 0) {
      printf("Cannot open mpu device file...\n");
      return -1;
    }
    return 0;
}

int get_accel(int16_t* values){
    return ioctl(fd, ACCEL, values);
}

void mpu_close(){
    close(fd);
}