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
	{ 0x3f4c09e, "misc_register" },
	{ 0x122c3a7e, "_printk" },
	{ 0x9f9a8a99, "misc_deregister" },
	{ 0xfe990052, "gpio_free" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xd9ec4b80, "gpio_to_desc" },
	{ 0xf0024008, "gpiod_direction_output_raw" },
	{ 0xdcb764ad, "memset" },
	{ 0x8c8569cb, "kstrtoint" },
	{ 0x178f2652, "gpiod_set_raw_value" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x39ff040a, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "C9975A42CCA819A3DE9730A");
