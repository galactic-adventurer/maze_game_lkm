obj-m += mpu6050.o

all:	mpu6050.ko

mpu6050.ko:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f mpu_dt.dtbo

mpu_dt.dtbo:
	dtc -I dts -O dtb -o mpu_dt.dtbo mpu_dt.dts

install: mpu_dt.dtbo mpu6050.ko
	sudo cp mpu_dt.dtbo /boot/overlays/
	sudo sh -c "echo 'dtoverlay=mpu_dt' >>/boot/config.txt"
	sudo insmod mpu6050.ko

uninstall:
	sudo rm -f /boot/overlays/mpu_dt.dtbo
	sudo nano /boot/config.txt
	sudo rmmod mpu6050.ko

