#include <linux/module.h>
#include <linux/fs.h>

#define DEV_NAME "mychardev"
static int major_num;

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    const char *msg = "Hello from kernel!\n";
    return simple_read_from_buffer(buf, len, offset, msg, strlen(msg));
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
};

static int __init char_dev_init(void)
{
    major_num = register_chrdev(0, DEV_NAME, &fops);
    if (major_num < 0) {
        pr_err("Device registration failed\n");
        return major_num;
    }
    pr_info("Major number: %d\n", major_num);
    return 0;
}

static void __exit char_dev_exit(void)
{
    unregister_chrdev(major_num, DEV_NAME);
    pr_info("char_dev removed\n");
}

module_init(char_dev_init);
module_exit(char_dev_exit);

MODULE_LICENSE("GPL");
