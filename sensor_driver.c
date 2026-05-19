#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/ktime.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#define TRIG 23
#define ECHO 24
#define BUZZER 18
#define LED 17

static struct timer_list ultra_timer;
static struct kobject *ultra_kobj;

static int distance = 0;
static int alert_state = 0;

/* ---------------- Distance Measurement ---------------- */
static int measure_distance(void)
{
    ktime_t start, end;
    s64 delta;
    int timeout = 50000;

    gpio_set_value(TRIG, 0);
    udelay(2);
    gpio_set_value(TRIG, 1);
    udelay(10);
    gpio_set_value(TRIG, 0);

    while (gpio_get_value(ECHO) == 0 && timeout--)
        cpu_relax();

    if (timeout <= 0)
        return -1;

    start = ktime_get();

    timeout = 50000;
    while (gpio_get_value(ECHO) == 1 && timeout--)
        cpu_relax();

    if (timeout <= 0)
        return -1;

    end = ktime_get();

    delta = ktime_to_us(ktime_sub(end, start));

    return delta / 15;
}

/* ---------------- Sysfs show ---------------- */
static ssize_t status_show(struct kobject *kobj,
                           struct kobj_attribute *attr,
                           char *buf)
{
    int d = distance;

    if (d < 0)
        return sprintf(buf, "ERR\n");

    if (d < 50)
        return sprintf(buf, "D:%d STOP\n", d);
    else
        return sprintf(buf, "D:%d CLEAR\n", d);
}

static struct kobj_attribute status_attr =
__ATTR(status, 0444, status_show, NULL);

/* ---------------- Timer Function ---------------- */
static void ultra_timer_func(struct timer_list *t)
{
    int d = measure_distance();

    if (d > 0)
        distance = d;

    if (d > 0 && d < 50)
    {
        gpio_set_value(LED, 1);
        gpio_set_value(BUZZER, 1);
        alert_state = 1;
    }
    else
    {
        gpio_set_value(LED, 0);
        gpio_set_value(BUZZER, 0);
        alert_state = 0;
    }

    mod_timer(&ultra_timer, jiffies + msecs_to_jiffies(500));
}

/* ---------------- Init ---------------- */
static int __init ultra_init(void)
{
    int ret;

    printk(KERN_INFO "Ultrasonic Driver Init\n");

    ret = gpio_request(TRIG, "trig");
    ret |= gpio_request(ECHO, "echo");
    ret |= gpio_request(BUZZER, "buzzer");
    ret |= gpio_request(LED, "led");

    if (ret) {
        printk(KERN_ERR "GPIO request failed\n");
        return -1;
    }

    gpio_direction_output(TRIG, 0);
    gpio_direction_input(ECHO);
    gpio_direction_output(BUZZER, 0);
    gpio_direction_output(LED, 0);

    /* Create sysfs node */
    ultra_kobj = kobject_create_and_add("ultra", kernel_kobj);
    if (!ultra_kobj) {
        printk(KERN_ERR "kobject creation failed\n");
        return -ENOMEM;
    }

    ret = sysfs_create_file(ultra_kobj, &status_attr.attr);
    if (ret) {
        printk(KERN_ERR "sysfs create failed\n");
        kobject_put(ultra_kobj);
        return ret;
    }

    timer_setup(&ultra_timer, ultra_timer_func, 0);
    mod_timer(&ultra_timer, jiffies + msecs_to_jiffies(500));

    printk(KERN_INFO "Ultrasonic Driver Loaded SUCCESS\n");

    return 0;
}

/* ---------------- Exit ---------------- */
static void __exit ultra_exit(void)
{
    del_timer_sync(&ultra_timer);

    gpio_set_value(LED, 0);
    gpio_set_value(BUZZER, 0);

    gpio_free(TRIG);
    gpio_free(ECHO);
    gpio_free(BUZZER);
    gpio_free(LED);

    sysfs_remove_file(ultra_kobj, &status_attr.attr);
    kobject_put(ultra_kobj);

    printk(KERN_INFO "Ultrasonic Driver Removed\n");
}

module_init(ultra_init);
module_exit(ultra_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Ultrasonic driver with LED + buzzer + sysfs ultra/status");
