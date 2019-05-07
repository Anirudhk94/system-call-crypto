#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
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
__used
__attribute__((section("__versions"))) = {
	{ 0x5a6028c0, "module_layout" },
	{ 0x1d47defc, "sysptr" },
	{ 0x449ad0a7, "memcmp" },
	{ 0x9f581d67, "vfs_write" },
	{ 0xc015eb73, "crypto_shash_digest" },
	{ 0x1a219599, "crypto_alloc_shash" },
	{ 0x7ad565ab, "vfs_read" },
	{ 0xa1d0a445, "vfs_unlink" },
	{ 0x69acdf38, "memcpy" },
	{ 0xf47e7cd6, "filp_open" },
	{ 0xfef8cf74, "vfs_statx" },
	{ 0x754d539c, "strlen" },
	{ 0xb2be2bd6, "getname" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xd2b09ce5, "__kmalloc" },
	{ 0x6718d62, "current_task" },
	{ 0x3c71ad4b, "filp_close" },
	{ 0x576567f8, "putname" },
	{ 0x37a0cba, "kfree" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0x1753456a, "kmem_cache_alloc_trace" },
	{ 0x369437a0, "kmalloc_caches" },
	{ 0xda4b9780, "unlock_rename" },
	{ 0xe8b7d223, "vfs_rename" },
	{ 0xd85c69b1, "lock_rename" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xff24c181, "crypto_destroy_tfm" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xc5850110, "printk" },
	{ 0x56f24f02, "crypto_alloc_base" },
	{ 0xb320cc0e, "sg_init_one" },
	{ 0xbdfb6dbb, "__fentry__" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "0B3EB5754EE58E4D76B4B8D");
