#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

#define DEVICE_NAME "lcd_button_char_dev"
#define DEVICE_COUNT 1

static struct cdev lcd_buttons_cdev;
static struct class *lcd_buttons_class;
static dev_t dev_number;
static wait_queue_head_t wait_queue;
static int button_pressed = 0;
static int last_button = 0;

// File operations for character device
static ssize_t read_button_number(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    char msg[2];
    int ret;

    wait_event_interruptible(wait_queue, button_pressed);
    button_pressed = 0;

    snprintf(msg, sizeof(msg), "%d", last_button);
    ret = copy_to_user(buffer, msg, strlen(msg) + 1);
    if (ret) {
        return -EFAULT;
    }

    return strlen(msg);
}

static struct file_operations fops = {
    .read = read_button_number,
};

// ISR for button interrupts
static irqreturn_t button_isr(int irq, void *dev_id) {
    last_button = *(int *)dev_id; // Retrieve button ID
    printk(KERN_INFO "Received interrupt for button #%d\n", last_button);
    button_pressed = 1;          // Notify wait queue
    wake_up_interruptible(&wait_queue);
    return IRQ_HANDLED;
}

struct drv_data {
    int button_id;
    int gpio;
};

static int button_probe(struct platform_device *pdev) {
    struct device *dev = &pdev->dev;
    struct gpio_desc *gpio_descr;
    struct drv_data *dev_data;
    int irq, ret;

 
    gpio_descr = gpiod_get(dev, "lcd-button", GPIOD_IN);
	if(IS_ERR(gpio_descr)) {
		dev_err(dev, "Could not setup the GPIO\n");
		return -1 * IS_ERR(gpio_descr);
	}

    gpiod_set_debounce(gpio_descr, 200);
    // Allocate memory for device data
    dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL); 
    if (!dev_data) { 
        gpiod_put(gpio_descr);
        dev_err(dev, "Could not allocate memory for device data\n");
        return -ENOMEM;
    }
    dev_data->gpio = desc_to_gpio(gpio_descr);

    ret = device_property_read_u32(dev, "id", &dev_data->button_id);
	if(ret) {
		dev_err(dev, "Could not read 'label'\n");
		goto err;
	}

    // Map GPIO to IRQ
    irq = gpio_to_irq(dev_data->gpio);
    if (irq < 0) {
        dev_err(dev, "Failed to get IRQ for GPIO %d\n", dev_data->gpio);
        ret = irq;
        goto err;
    }

    // Register interrupt handler
    ret = request_irq(irq, button_isr, IRQF_TRIGGER_FALLING, dev_name(dev), dev_data);
    if (ret) {
        dev_err(dev, "Failed to request IRQ %d for GPIO %d\n", irq, dev_data->gpio);
        goto err;
    }

    platform_set_drvdata(pdev, dev_data);
    dev_info(dev, "Button driver probed for GPIO %d, IRQ %d\n", dev_data->gpio, irq);
    return 0;
err:
    gpiod_put(gpio_descr);
    return ret;
}


static void button_remove(struct platform_device *pdev) {
    struct device *dev = &pdev->dev;
    struct drv_data *dev_data= platform_get_drvdata(pdev);
    free_irq(gpio_to_irq(dev_data->gpio), dev_data);
    gpiod_put(gpio_to_desc(dev_data->gpio));
    dev_info(dev, "Button device removed\n");
}



static const struct of_device_id button_dt_ids[] = {
    { .compatible = "lcd_button" },
    { /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, button_dt_ids);

static struct platform_driver button_driver = {
    .probe = button_probe,
    .remove = button_remove,
    .driver = {
        .name = "lcd_button_driver",
        .of_match_table = button_dt_ids,
    },
};


static int __init button_init(void) {
    int ret;

    init_waitqueue_head(&wait_queue);

    // Register platform driver
    ret = platform_driver_register(&button_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register platform driver\n");
        return ret;
    }

    // Allocate character device
    ret = alloc_chrdev_region(&dev_number, 0, DEVICE_COUNT, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate character device region\n");
        platform_driver_unregister(&button_driver);
        return ret;
    }

    cdev_init(&lcd_buttons_cdev, &fops);
    ret = cdev_add(&lcd_buttons_cdev, dev_number, DEVICE_COUNT);
    if (ret < 0) {
        printk(KERN_ERR "Failed to add character device\n");
        goto err;
    }

    // Create sysfs class
    lcd_buttons_class = class_create(DEVICE_NAME);
    if (IS_ERR(lcd_buttons_class)) {
        cdev_del(&lcd_buttons_cdev);
        printk(KERN_ERR "Cannot create sysfs class\n");
        goto err;
    }
    if (IS_ERR(
            device_create(lcd_buttons_class, NULL, dev_number, NULL, DEVICE_NAME))) {
        printk(KERN_ERR "Cannot create device in sysfs\n");
        cdev_del(&lcd_buttons_cdev);
        class_destroy(lcd_buttons_class);
        goto err;
    }

    

    printk(KERN_INFO "LCD button driver loaded\n");
    return 0;
err:
    platform_driver_unregister(&button_driver);
    unregister_chrdev_region(dev_number, DEVICE_COUNT);
    return -1;
}

static void __exit button_exit(void) {
    cdev_del(&lcd_buttons_cdev);
    device_destroy(lcd_buttons_class, dev_number);
    class_destroy(lcd_buttons_class);
    unregister_chrdev_region(dev_number, DEVICE_COUNT);
    platform_driver_unregister(&button_driver);
    printk(KERN_INFO "LCD button driver unloaded\n");
}

module_init(button_init);
module_exit(button_exit);
