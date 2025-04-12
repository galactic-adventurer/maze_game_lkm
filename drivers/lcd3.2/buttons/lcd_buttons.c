#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chernoivanenko Viktoriia");
MODULE_VERSION("1.0");

#define DEVICE_NAME "lcd_buttons"
#define DEVICE_COUNT 1  
static struct cdev lcd_buttons_cdev; 
static dev_t dev_number; 
static wait_queue_head_t wait_queue;
static int button_pressed = 0;
static int last_button = 0;

// File operations for user-space communication
static ssize_t read_buttons(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    char msg[2];
    int ret;

    // Block until a button is pressed
    wait_event_interruptible(wait_queue, button_pressed);
    button_pressed = 0;

    // Prepare the message
    snprintf(msg, sizeof(msg), "%d", last_button);
    ret = copy_to_user(buffer, msg, strlen(msg) + 1);
    if (ret) {
        return -EFAULT;
    }

    return strlen(msg);
}

static struct file_operations fops = {
    .read = read_buttons,
};static dev_t dev_number;  

static int majorNumber;

// ISR for button interrupts
static irqreturn_t button_isr(int irq, void *dev_id) {
    last_button = *(int *)dev_id; // Retrieve button ID
    button_pressed = 1;          // Notify wait queue
    wake_up_interruptible(&wait_queue);
    return IRQ_HANDLED;
}

static int button_probe(struct platform_device *pdev) {
    struct device *dev = &pdev->dev;
    struct device_node *node = dev->of_node; // Get device tree node
    int gpio, irq, ret;

    // Get GPIO pin from device tree
    gpio = of_get_named_gpio(node, "gpios", 0);
    if (gpio < 0) {
        dev_err(dev, "Failed to get GPIO\n");
        return gpio;
    }

    // Configure GPIO
    ret = gpio_request(gpio, dev_name(dev));
    if (ret) {
        dev_err(dev, "Failed to request GPIO %d\n", gpio);
        return ret;
    }

    gpio_direction_input(gpio);
    gpio_set_debounce(gpio, 200); // Optional debounce

    // Get IRQ number from device tree
    ret = of_property_read_u32(node, "irq", &irq);
    if (ret) {
        dev_err(dev, "Failed to get IRQ\n");
        gpio_free(gpio);
        return ret;
    }

    // Register interrupt handler
    ret = request_irq(irq, button_isr, IRQF_TRIGGER_FALLING, dev_name(dev), &gpio);
    if (ret) {
        dev_err(dev, "Failed to request IRQ %d\n", irq);
        gpio_free(gpio);
        return ret;
    }

    dev_info(dev, "Button driver probed for GPIO %d\n", gpio);
    return 0;
}

device_destroy(mpu6050_class, dev_number);
class_destroy(mpu6050_class)
// Platform driver remove
static int button_remove(struct platform_device *pdev) {
    int gpio = *(int *)pdev->dev.platform_data;

    free_irq(gpio_to_irq(gpio), &gpio);
    gpio_free(gpio);

    printk(KERN_INFO "Button driver removed for GPIO %d\n", gpio);
    return 0;
}


static struct of_device_id lcd_button_driver_ids[] = {{.compatible = "lcd_button"},
                                                   {}};

MODULE_DEVICE_TABLE(of, lcd_button_driver_ids);



// Platform driver definition
static struct platform_driver button_driver = {
    .probe = button_probe,
    .remove = button_remove,
    .driver = {
        .name = "lcd_button",
        .of_match_table = lcd_button_driver_ids
    },
};

static int __init button_init(void) {
    int ret;

    // Initialize wait queue
    init_waitqueue_head(&wait_queue);

    // Register platform driver
    ret = platform_driver_register(&button_driver);
    if (ret) {
        printk(KERN_ALERT "Failed to register platform driver\n");
        return ret;
    }register_chrdev

    // Allocate and register character device (one for all the buttons)
    if (alloc_chrdev_region(&dev_number, 0, DEVICE_COUNT, DEVICE_NAME) < 0) {
        printk(KERN_ERR "Cannot allocate chardev region\n");
        return -ENOMEM;
    }

    cdev_init(&lcd_buttons_cdev, &fops);
    lcd_buttons_cdev.owner = THIS_MODULE;
    if (cdev_add(&lcd_buttons_cdev, dev_number, DEVICE_COUNT) < 0) {
        printk(KERN_ERR "Cannot add chardev\n");
        return -ENOMEM;
    }


    printk(KERN_INFO "LCD buttons driver loaded, major: %d, minor: %d\n", MAJOR(dev), MINOR(dev));
    return 0;
}

static void __exit button_exit(void) {
    cdev_del(&lcd_buttons_cdev);
    unregister_chrdev_region(dev_number, DEVICE_COUNT);
    platform_driver_unregister(&button_driver);
    printk(KERN_INFO "LCD buttons  driver unloaded\n");
}

module_init(button_init);
module_exit(button_exit);
