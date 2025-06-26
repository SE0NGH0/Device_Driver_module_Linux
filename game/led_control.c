/*
 * led_control.c - Linux misc-driver for 3 LEDs on GPIOs 17, 27, 22
 * with deferred-init workaround and major/minor log on load
 *
 * Usage: echo <mask> > /dev/led_control
 * mask: bit0 -> LED on GPIO17
 *       bit1 -> LED on GPIO27
 *       bit2 -> LED on GPIO22
 * Example: mask=5 (0b101) -> GPIO17=1, GPIO27=0, GPIO22=1
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define DEVICE_NAME "led_control"

static int leds[] = {17, 27, 22};
static const int num_leds = ARRAY_SIZE(leds);
static bool gpio_initialized;

// One-time GPIO setup at first write
static int init_gpios(void)
{
    int ret;
    int i;

    for (i = 0; i < num_leds; i++) {
        ret = gpio_request(leds[i], DEVICE_NAME);
        if (ret) {
            pr_err("%s: gpio_request(%d) failed: %d\n",
                   DEVICE_NAME, leds[i], ret);
            while (--i >= 0)
                gpio_free(leds[i]);
            return ret;
        }
        ret = gpio_direction_output(leds[i], 0);
        if (ret) {
            pr_err("%s: gpio_direction_output(%d) failed: %d\n",
                   DEVICE_NAME, leds[i], ret);
            gpio_free(leds[i]);
            while (--i >= 0)
                gpio_free(leds[i]);
            return ret;
        }
    }

    gpio_initialized = true;
    pr_info("%s: GPIOs initialized\n", DEVICE_NAME);
    return 0;
}

// Write handler: initialize GPIOs on first write and set LEDs
static ssize_t led_write(struct file *file, const char __user *buf,
                         size_t count, loff_t *ppos)
{
    char kbuf[4] = {0};
    int mask;
    size_t len = min(count, sizeof(kbuf) - 1);
    int ret;
    int i;

    if (!gpio_initialized) {
        ret = init_gpios();
        if (ret)
            return ret;
    }

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;
    kbuf[len] = '\0';

    if (kstrtoint(kbuf, 10, &mask))
        return -EINVAL;

    mask &= 0x7; // only 3 bits
    for (i = 0; i < num_leds; i++)
        gpio_set_value(leds[i], (mask >> i) & 0x1);

    return count;
}

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .write = led_write,
};

static struct miscdevice led_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &led_fops,
};

static int __init led_init(void)
{
    int ret;
    struct device *dev;

    ret = misc_register(&led_misc);
    if (ret) {
        pr_err("%s: misc_register failed: %d\n", DEVICE_NAME, ret);
        return ret;
    }

    dev = led_misc.this_device;
    pr_info("%s: Module loaded, device /dev/%s (major=%d, minor=%d)\n",
            DEVICE_NAME, DEVICE_NAME,
            MAJOR(dev->devt), MINOR(dev->devt));
    return 0;
}

static void __exit led_exit(void)
{
    misc_deregister(&led_misc);
    if (gpio_initialized) {
        int j;
        for (j = 0; j < num_leds; j++)
            gpio_free(leds[j]);
        pr_info("%s: GPIOs freed\n", DEVICE_NAME);
    }
    pr_info("%s: Module unloaded\n", DEVICE_NAME);
}

module_init(led_init);
module_exit(led_exit);

MODULE_AUTHOR("hotari");
MODULE_DESCRIPTION("Misc driver for 3 GPIO LEDs with deferred-init workaround and MAJOR/MINOR log");
MODULE_LICENSE("GPL");
