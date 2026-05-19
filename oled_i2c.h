#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/delay.h>

#define OLED_ADDR 0x3C
#define OLED_I2C_DEVICE_NAME "oled_file"
