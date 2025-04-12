#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include "ili9341.h"
#include "ili9341_config.h"

#define DEVICE_NAME "lcd_display"
#define DEVICE_COUNT 1

#define DATA_SIZE	90

static u16 frame_buffer[LCD_WIDTH * LCD_HEIGHT];
static struct class *lcd_display_class = NULL;
static struct cdev lcd_cdev; 
static struct spi_device *spi_dev = NULL;
static dev_t dev_number; 

inline void lcd_update_screen(void)
{
	lcd_ili9341_write_data((u8*)frame_buffer, sizeof(frame_buffer));
}

static ssize_t lcd_cdev_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    size_t to_copy = min(len, sizeof(frame_buffer)) - (int) (*offset);
    
    if (copy_to_user(buffer, frame_buffer + *offset, to_copy))
        return -EFAULT;

    offset += to_copy
    return to_copy;
}

static ssize_t lcd_cdev_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    size_t to_copy = min(len, sizeof(frame_buffer)) - (int) (*offset);

    if (copy_from_user(frame_buffer, buffer, to_copy))
        return -EFAULT;

    offset += to_copy

    lcd_update_screen();

    return to_copy;
}

static struct file_operations fops = {
    .read = lcd_cdev_read,
    .write = lcd_cdev_write,
};

static int lcd_spi_probe(struct spi_device *spi) {
    spi_dev = spi;

    // Create chardev
    if (alloc_chrdev_region(&dev_number, 0, DEVICE_COUNT, DEVICE_NAME) < 0) {
        printk(KERN_ERR "Cannot allocate chardev region\n");
        return -ENOMEM;
    }

    cdev_init(&lcd_cdev, &fops);
    lcd_cdev.owner = THIS_MODULE;
    if (cdev_add(&lcd_cdev, dev_number, DEVICE_COUNT) < 0) {
        printk(KERN_ERR "Cannot add chardev\n");
        goto err;
    }

    // Create sysfs class
    lcd_display_class = class_create(DEVICE_NAME);
    if (IS_ERR(lcd_display_class)) {
        cdev_del(&mpu6050_cdev);
        printk(KERN_ERR "Cannot create sysfs class\n");
        goto err;
    }

    // Create device in /dev
    static struct device *spiDevice = device_create(spiClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(spiDevice)) {
        class_destroy(spiClass);
        printk(KERN_ERR "Cannot create device in sysfs\n");
        goto err;
    }

    // Initialize lcd display
    lcd_ili9341_init();

    printk(KERN_INFO "LCD SPI probed successfully\n");
    return 0;

err:
    
    unregister_chrdev_region(dev_number, DEVICE_COUNT);
    return -1;
 
}

static int lcd_spi_remove(struct spi_device *spi) {
    device_destroy(lcd_display_class, dev_number);
    class_destroy(lcd_display_class);
    cdev_del(&lcd_cdev);
    unregister_chrdev_region(dev_number, DEVICE_COUNT);
    printk(KERN_INFO "LCD SPI Device driver unregistered\n");
    return 0;
}

static const struct of_device_id spi_of_match[] = {
    { .compatible = "lcd_ili9341" },
    {}
};
MODULE_DEVICE_TABLE(of, spi_of_match);

static struct spi_driver spi_driver = {
    .driver = {
        .name = DEVICE_NAME,
        .owner = THIS_MODULE,
        .of_match_table = spi_of_match,
    },
    .probe = lcd_spi_probe,
    .remove = lcd_spi_remove,
};

module_spi_driver(spi_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chernoivanenko Viktoriia");
MODULE_VERSION("1.0");
