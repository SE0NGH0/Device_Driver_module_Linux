#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define DEV_NAME "led-driver"
#define BCM_BASE 0xFE000000
#define GPIO_BASE (BCM_BASE + 0x200000)

#define GPIO_PIN 18

static void __iomem *gpio_base;
#define INP_GPIO(g)   (*(volatile unsigned int *)(gpio_base + ((g)/10)*4) &= ~(7 << (((g)%10)*3)))
#define OUT_GPIO(g)   (*(volatile unsigned int *)(gpio_base + ((g)/10)*4) |=  (1 << (((g)%10)*3)))
#define GPIO_SET(g)   (*(volatile unsigned int *)(gpio_base + 0x1C) = (1 << g))
#define GPIO_CLR(g)   (*(volatile unsigned int *)(gpio_base + 0x28) = (1 << g))

static int major;

static ssize_t myled_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char kbuf[2] = {0};

    if (copy_from_user(kbuf, buf, 1)) return -EFAULT;

    if (kbuf[0] == '1') {
           GPIO_SET(GPIO_PIN);  // 켜기
           pr_info("LED ON\n");
        }
    else if (kbuf[0] == '0') {
         GPIO_CLR(GPIO_PIN);  // 끄기
        pr_info("LED OFF\n");
}

    return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = myled_write,
};

static int __init myled_init(void)
{
    major = register_chrdev(0, DEV_NAME, &fops);
    if (major < 0) return major;

    gpio_base = ioremap(GPIO_BASE, 0x100);
    INP_GPIO(GPIO_PIN);
    OUT_GPIO(GPIO_PIN);

    pr_info("myled loaded: /dev/%s (major %d)\n", DEV_NAME, major);
    return 0;
}

static void __exit myled_exit(void)
{
    unregister_chrdev(major, DEV_NAME);
    if (gpio_base) iounmap(gpio_base);
    pr_info("myled unloaded\n");
}

module_init(myled_init);
module_exit(myled_exit);
MODULE_LICENSE("GPL");
