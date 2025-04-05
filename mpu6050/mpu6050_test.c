#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
 
#define RD_VALUE _IOR('m','g',int16_t*)
 
int main()
{
        int fd;
        int16_t gyro_values[3];
        printf("\nOpening Driver\n");
        fd = open("/dev/mpu6050", O_RDONLY);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }
 
        printf("Reading Value from Driver\n");
        ioctl(fd, GYRO, (int16_t*) &gyro_values);
        printf("Gyro values:\n");
        for (int i; i<3; i++){
            printf("%d ", gyro_values[i]);
        }
        printf("Closing Driver\n");
        close(fd);
}