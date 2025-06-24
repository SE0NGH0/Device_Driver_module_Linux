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
	{ 0x5151c2fc, "sysfs_remove_group" },
	{ 0x5b08cec9, "i2c_del_driver" },
	{ 0xbb41b493, "i2c_smbus_write_byte_data" },
	{ 0xf9a482f9, "msleep" },
	{ 0xcc4fbae3, "i2c_smbus_read_byte_data" },
	{ 0x7eb87600, "devm_kmalloc" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0x4dee97b2, "i2c_smbus_read_i2c_block_data" },
	{ 0x8950dd19, "sysfs_create_group" },
	{ 0x1a283f39, "_dev_info" },
	{ 0xf810f451, "_dev_err" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x39ff040a, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:bmp180");
MODULE_ALIAS("of:N*T*Cbosch,bmp180");
MODULE_ALIAS("of:N*T*Cbosch,bmp180C*");

MODULE_INFO(srcversion, "C7B70403AE887C12A104005");
