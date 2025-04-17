obj-m := lcd_display.o
lcd_display-objs := ili9341.o lcd_screen.o

all:	lcd_display.ko

lcd_display.ko:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f lcd_screen.dtbo

lcd_screen.dtbo:
	dtc -I dts -O dtb -o lcd_screen.dtbo lcd_screen.dts

install: lcd_screen.dtbo lcd_display.ko
	sudo cp lcd_screen.dtbo /boot/overlays/
	sudo sh -c "echo 'dtoverlay=lcd_screen' >>/boot/config.txt"
	sudo insmod lcd_display.ko
	sudo overlay lcd_screen

uninstall:
	sudo rm -f /boot/overlays/lcd_screen.dtbo
	sudo nano /boot/config.txt
	sudo rmmod lcd_display.ko
	sudo rmdir /sys/kernel/config/device-tree/overlays/*lcd_screen*
	

