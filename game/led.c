/*
 * led_control.c  -  Linux misc-driver for 3 LEDs
 *
 * Usage: echo <n> > /dev/led_control
 *   n = 0  -> light LED0
 *   n = 1  -> light LED1
 *   n = 2  -> light LED2
 *   any other -> all LEDs off
 *
 * GPIO pins (BCM numbering):
 *   LED0 -> 17
 *   LED1 -> 27
 *   LED2 -> 22
 */
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define LED0_GPIO 17
#define LED1_GPIO 27
#define LED2_GPIO 22

static ssize_t led_write(struct file *file,
                         const char __user *buf,
                         size_t count,
                         loff_t *ppos)
{
    char kbuf[4] = {0};
    int n;

    if (count < 1)
        return -EINVAL;
    if (count > sizeof(kbuf) - 1)
        count = sizeof(kbuf) - 1;
    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;

    n = kbuf[0] - '0';

    /* turn off all first */
    gpio_set_value(LED0_GPIO, 0);
    gpio_set_value(LED1_GPIO, 0);
    gpio_set_value(LED2_GPIO, 0);

    /* light selected LED */
    switch (n) {
    case 0: gpio_set_value(LED0_GPIO, 1); break;
    case 1: gpio_set_value(LED1_GPIO, 1); break;
    case 2: gpio_set_value(LED2_GPIO, 1); break;
    default: /* all off */; break;
    }

    return count;
}

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .write = led_write,
};

static struct miscdevice led_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "led_control",
    .fops = &led_fops,
};

static int __init led_init(void)
{
    int ret;

    /* request GPIOs */
    gpio_request(LED0_GPIO, "LED0");
    gpio_direction_output(LED0_GPIO, 0);
    gpio_request(LED1_GPIO, "LED1");
    gpio_direction_output(LED1_GPIO, 0);
    gpio_request(LED2_GPIO, "LED2");
    gpio_direction_output(LED2_GPIO, 0);

    /* register misc device */
    ret = misc_register(&led_dev);
    if (ret) {
        pr_err("led_control: misc_register failed\n");
        return ret;
    }
    pr_info("led_control: registered /dev/led_control\n");
    return 0;
}

static void __exit led_exit(void)
{
    misc_deregister(&led_dev);
    gpio_set_value(LED0_GPIO, 0);
    gpio_set_value(LED1_GPIO, 0);
    gpio_set_value(LED2_GPIO, 0);
    gpio_free(LED0_GPIO);
    gpio_free(LED1_GPIO);
    gpio_free(LED2_GPIO);
    pr_info("led_control: unloaded\n");
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Misc driver to control 3 LEDs via /dev/led_control");
