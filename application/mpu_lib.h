#include <sys/ioctl.h>
#include <stdint.h>
#define MPU_DEV_FILE "/dev/mpu6050"
#define ACCEL _IOR('m', 'a', int16_t *)


int mpu_open();
int get_accel(int16_t* values);
void mpu_close();


