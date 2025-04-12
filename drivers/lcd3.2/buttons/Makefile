obj-m += lcd_buttons.o

all:	lcd_buttons.ko

lcd_buttons.ko:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f lcd_buttons.dtbo

lcd_buttons.dtbo:
	dtc -I dts -O dtb -o lcd_buttons.dtbo lcd_buttons.dts

install: lcd_buttons.dtbo lcd_buttons.ko
	sudo cp lcd_buttons.dtbo /boot/overlays/
	sudo sh -c "echo 'dtoverlay=lcd_buttons' >>/boot/config.txt"
	sudo insmod lcd_buttons.ko

uninstall:
	sudo rm -f /boot/overlays/lcd_buttons.dtbo
	sudo nano /boot/config.txt
	sudo rmmod lcd_buttons.ko

