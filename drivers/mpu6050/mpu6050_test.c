#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ACCEL _IOR('m', 'a', int16_t *)

int main() {
  int fd;
  int16_t accel_values[3];
  printf("\nOpening Driver\n");
  fd = open("/dev/mpu6050", O_RDONLY);
  if (fd < 0) {
    printf("Cannot open device file...\n");
    return 0;
  }

  printf("Reading Value from Driver\n");
  ioctl(fd, ACCEL, (int16_t *)&accel_values);
  printf("Accel values:\n");
  for (int i; i < 3; i++) {
    printf("%d ", accel_values[i]);
  }
  printf("Closing Driver\n");
  close(fd);
}
