#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xa6ef395c, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x4b37e7bc, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0x6bf1c17f, __VMLINUX_SYMBOL_STR(pv_lock_ops) },
	{ 0x3783f03a, __VMLINUX_SYMBOL_STR(framebuffer_release) },
	{ 0x487f831, __VMLINUX_SYMBOL_STR(fb_find_best_display) },
	{ 0x7d9f8ab1, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0xa9a8e17f, __VMLINUX_SYMBOL_STR(arch_phys_wc_add) },
	{ 0x614e0821, __VMLINUX_SYMBOL_STR(cfb_fillrect) },
	{ 0x85df9b6c, __VMLINUX_SYMBOL_STR(strsep) },
	{ 0x9d7813aa, __VMLINUX_SYMBOL_STR(fb_set_suspend) },
	{ 0xc631580a, __VMLINUX_SYMBOL_STR(console_unlock) },
	{ 0xc0e833f1, __VMLINUX_SYMBOL_STR(param_ops_charp) },
	{ 0xc1680c0f, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0xfb578fc5, __VMLINUX_SYMBOL_STR(memset) },
	{ 0x31d4c4b1, __VMLINUX_SYMBOL_STR(pci_restore_state) },
	{ 0xacc797bf, __VMLINUX_SYMBOL_STR(cfb_imageblit) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x7a890c8, __VMLINUX_SYMBOL_STR(fb_alloc_cmap) },
	{ 0xe531acf2, __VMLINUX_SYMBOL_STR(register_framebuffer) },
	{ 0x5a921311, __VMLINUX_SYMBOL_STR(strncmp) },
	{ 0xfbaaf01e, __VMLINUX_SYMBOL_STR(console_lock) },
	{ 0x5792f848, __VMLINUX_SYMBOL_STR(strlcpy) },
	{ 0x5e7544a0, __VMLINUX_SYMBOL_STR(platform_device_unregister) },
	{ 0xa598e29c, __VMLINUX_SYMBOL_STR(vesa_modes) },
	{ 0x61651be, __VMLINUX_SYMBOL_STR(strcat) },
	{ 0x1dcd6333, __VMLINUX_SYMBOL_STR(platform_device_register) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0xaa5433ac, __VMLINUX_SYMBOL_STR(fb_find_mode) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xfcfa03ff, __VMLINUX_SYMBOL_STR(fb_videomode_to_modelist) },
	{ 0x71a59478, __VMLINUX_SYMBOL_STR(pv_cpu_ops) },
	{ 0xff9ca065, __VMLINUX_SYMBOL_STR(fb_edid_to_monspecs) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0x58262f8a, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0x1f9651ad, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xe259ae9e, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0x4733a49c, __VMLINUX_SYMBOL_STR(pci_set_power_state) },
	{ 0xbd67e26c, __VMLINUX_SYMBOL_STR(framebuffer_alloc) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0x868abd9a, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0x5313636c, __VMLINUX_SYMBOL_STR(cfb_copyarea) },
	{ 0xc6ce21a2, __VMLINUX_SYMBOL_STR(pci_choose_state) },
	{ 0x50d68377, __VMLINUX_SYMBOL_STR(arch_phys_wc_del) },
	{ 0x59a3a22f, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x36330503, __VMLINUX_SYMBOL_STR(pci_save_state) },
	{ 0xffd88f98, __VMLINUX_SYMBOL_STR(unregister_framebuffer) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v0000126Fd00000750sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000126Fd00000718sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000126Fd00000712sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000126Fd00000720sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000126Fd00000501sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "00F3B0B4A03D5294BDCCA70");
