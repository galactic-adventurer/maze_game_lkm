#include "mpu6050.h"
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chernoivanenko Viktoriia");
MODULE_VERSION("1.0");

/* Define output data structure */

struct mpu_data {
  struct i2c_client *client;
  int16_t accel_values[3];
  int16_t gyro_values[3];
  int16_t temperature;
};

static struct mpu_data mpu6050_data;

/* Define options and variables for dynamic chardev registering by udev*/
#define DEVICE_NAME "mpu6050"
#define DEVICE_COUNT 1              // number of devices that can be registered
static struct class *mpu6050_class; // for class in sysfs
static struct cdev mpu6050_cdev;    // for chardev
static dev_t dev_number;            // for major and minor numbers

/* Define objects for reading thread*/
static struct task_struct *accel_read_thread;
static struct mutex accel_lock;

/* Implement reading functions from i2c device*/

static int mpu6050_read_accel(void *args) {
  if (mpu6050_data.client == 0) {
    return -ENODEV;
  }

  while (!kthread_should_stop()) {
    mutex_lock(&accel_lock);
    mpu6050_data.accel_values[0] = (s16)((u16)i2c_smbus_read_word_swapped(
        mpu6050_data.client, REG_ACCEL_XOUT_H));
    mpu6050_data.accel_values[1] = (s16)((u16)i2c_smbus_read_word_swapped(
        mpu6050_data.client, REG_ACCEL_YOUT_H));
    mpu6050_data.accel_values[2] = (s16)((u16)i2c_smbus_read_word_swapped(
        mpu6050_data.client, REG_ACCEL_ZOUT_H));
    mutex_unlock(&accel_lock);

    mdelay(100);
  }

  return 0;
}

/* Declare probe and remove functions of i2c device driver inteface */

static int mpu6050_probe(struct i2c_client *client);
static void mpu6050_remove(struct i2c_client *client);

/* Set compatibility parameters */
static struct of_device_id mpu6050_driver_ids[] = {{.compatible = "mpu6050"},
                                                   {}};

MODULE_DEVICE_TABLE(of, mpu6050_driver_ids);

/* Create i2c driver structure */

static struct i2c_driver mpu6050_driver = {
    .probe = mpu6050_probe,
    .remove = mpu6050_remove,
    .driver = {.name = "mpu6050", .of_match_table = mpu6050_driver_ids},
};

/* Implementation of ioctl interfile_operationsface to user */
#define ACCEL _IOR('m', 'a', int16_t *)
#define GYRO _IOR('m', 'g', int16_t *)
#define TEMP _IOR('m', 't', int16_t *)

static long mpu6050_ioctl(struct file *file, unsigned int cmd,
                          unsigned long arg) {
  switch (cmd) {
  case ACCEL:
    int ret;
    mutex_lock(&accel_lock);
    ret = copy_to_user((int16_t *)arg, &mpu6050_data.accel_values,
                       sizeof(mpu6050_data.accel_values));
    printk(KERN_INFO "sensor data read:\n");
    printk(KERN_INFO "ACCEL[X,Y,Z] = [%d, %d, %d]\n",
           mpu6050_data.accel_values[0], mpu6050_data.accel_values[1],
           mpu6050_data.accel_values[2]);
    mutex_unlock(&accel_lock);
    if (ret) {
      pr_err("ACCEL READ : Err!\n");
    }
    break;

  default:
    pr_info("Default\n");
    break;
  }
  return 0;
}

static struct file_operations mpu6050_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = mpu6050_ioctl,
};

/* Implementation module functionality on uploading and unloading */
/**))

 * @brief This function is called on loading the driver
 */
static int mpu6050_probe(struct i2c_client *client) {

  mpu6050_data.client = client;

  printk(KERN_INFO "i2c client address is 0x%X\n", client->addr);

  // Create chardev
  if (alloc_chrdev_region(&dev_number, 0, DEVICE_COUNT, DEVICE_NAME) < 0) {
    printk(KERN_ERR "Cannot allocate chardev region\n");
    return -ENOMEM;
  }

  cdev_init(&mpu6050_cdev, &mpu6050_fops);
  mpu6050_cdev.owner = THIS_MODULE;
  if (cdev_add(&mpu6050_cdev, dev_number, DEVICE_COUNT) < 0) {
    printk(KERN_ERR "Cannot add chardev\n");
    goto err;
  }

  // Create sysfs class
  mpu6050_class = class_create(DEVICE_NAME);
  if (IS_ERR(mpu6050_class)) {
    cdev_del(&mpu6050_cdev);
    printk(KERN_ERR "Cannot create sysfs class\n");
    goto err;
  }
  if (IS_ERR(
          device_create(mpu6050_class, NULL, dev_number, NULL, DEVICE_NAME))) {
    printk(KERN_ERR "Cannot create device in sysfs\n");
    class_destroy(mpu6050_class);
    goto err;
  }

  /* Initial device setup */
  /* No error handling here! */
  i2c_smbus_write_byte_data(client, REG_CONFIG, 0);
  i2c_smbus_write_byte_data(client, REG_GYRO_CONFIG, 0);
  i2c_smbus_write_byte_data(client, REG_ACCEL_CONFIG, 0xE0);
  i2c_smbus_write_byte_data(client, REG_FIFO_EN, 0);
  i2c_smbus_write_byte_data(client, REG_INT_PIN_CFG, 0);
  i2c_smbus_write_byte_data(client, REG_INT_ENABLE, 0);
  i2c_smbus_write_byte_data(client, REG_USER_CTRL, 0);
  i2c_smbus_write_byte_data(client, REG_PWR_MGMT_1, 0);
  i2c_smbus_write_byte_data(client, REG_PWR_MGMT_2, 0);

  /* Create reading data thread */
  accel_read_thread =
      kthread_run(mpu6050_read_accel, NULL, "accelc_read_thread");
  if (accel_read_thread != NULL)
    printk(KERN_INFO "accel reading thread is running");
  else {
    printk(KERN_ERR "can't create gyro reading thread");
    return -1;
  }

  printk(KERN_INFO "i2c driver probed\n");
  return 0;

err:
  unregister_chrdev_region(dev_number, DEVICE_COUNT);
  return -1;
 
}

/**
 * @brief This function is called on unloading the driver
 */
static void mpu6050_remove(struct i2c_client *client) {
  kthread_stop(accel_read_thread);

  mpu6050_data.client = 0;

  device_destroy(mpu6050_class, dev_number);
  class_destroy(mpu6050_class);
  cdev_del(&mpu6050_cdev);
  unregister_chrdev_region(dev_number, DEVICE_COUNT);
  printk(KERN_ERR "i2c driver removed\n");
}

/* Automatic creation of init and exit function for i2c driver */
module_i2c_driver(mpu6050_driver);
