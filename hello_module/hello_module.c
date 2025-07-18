#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple Hello World module");

static int __init hello_init(void) {
    printk(KERN_INFO "Hello World from kernel module!\n");
    return 0;
}

static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye from kernel module!\n");
}

module_init(hello_init);
module_exit(hello_exit);
