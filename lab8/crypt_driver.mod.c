#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x122c3a7e, "_printk" },
	{ 0xc21bdf9, "device_destroy" },
	{ 0xc9484e52, "class_unregister" },
	{ 0xfa5cdaeb, "class_destroy" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xa916b694, "strnlen" },
	{ 0xe16fb92a, "crypto_alloc_skcipher" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xc213cb28, "crypto_skcipher_setkey" },
	{ 0x43babd19, "sg_init_one" },
	{ 0x230759c0, "crypto_skcipher_decrypt" },
	{ 0xd0760fc0, "kfree_sensitive" },
	{ 0x89463adb, "crypto_destroy_tfm" },
	{ 0xe1f1c75e, "crypto_skcipher_encrypt" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x2f920aea, "__register_chrdev" },
	{ 0x33c47e0e, "class_create" },
	{ 0xf641a686, "device_create" },
	{ 0xb2b23fc2, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "D5E1E919C8C1AC8BE47D3A9");
