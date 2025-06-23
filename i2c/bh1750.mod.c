#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xeb081dc5, "i2c_register_driver" },
	{ 0xe095e43a, "device_destroy" },
	{ 0x607587f4, "cdev_del" },
	{ 0x4a41ecb3, "class_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x7eb87600, "devm_kmalloc" },
	{ 0x5d9d9fd4, "cdev_init" },
	{ 0xcc335c1c, "cdev_add" },
	{ 0x93ab9e33, "device_create" },
	{ 0x1a283f39, "_dev_info" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xf311fc60, "class_create" },
	{ 0x5b08cec9, "i2c_del_driver" },
	{ 0x61f9c02a, "i2c_smbus_write_byte" },
	{ 0xf9a482f9, "msleep" },
	{ 0x4dee97b2, "i2c_smbus_read_i2c_block_data" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x39ff040a, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:bh1750");

MODULE_INFO(srcversion, "C523613AD956B3A12139285");
