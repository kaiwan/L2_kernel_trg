/*
 * slab_custom.c
 *
 * Simple demo of creating a custom slab cache.
 * Notice how the kernel reuses an existing cache if your cache size is a
 * close-enough match!
 * Load this module & look up /proc/slabinfo ..
 *
 * (c) Kaiwan NB
 * License: MIT
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include "../../convenient.h"

#define DRVNAME "slab_custom"
#define MYBUFSZ 350		//-- using size '350' seems to give us a new private slab cache

static struct kmem_cache *my_cachep;

struct MyStruct {
	u32 a, b, c;
	s8 *cptr;
	u16 valid;
	u8 intbuf[MYBUFSZ];
};

static void mycache_use(void)
{
	void *object;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)
	pr_info("Cache name is %s\n", kmem_cache_name(my_cachep));
#else
	pr_info("[ker ver > 2.6.38 cache name deprecated...]\n");
#endif
	pr_info("Cache object size is %u\n", kmem_cache_size(my_cachep));

	object = kmem_cache_alloc(my_cachep, GFP_KERNEL);

	memset(object, 0xae, sizeof(struct MyStruct));
	print_hex_dump_bytes(" ", DUMP_PREFIX_ADDRESS, object, sizeof(struct MyStruct));

	kmem_cache_free(my_cachep, object);
}

static void init_my_cache(void)
{
#if 0
	char *snum = "1002";
	int n;

	// just for flawfinder etc demo :-p
	n = kstrtoint(snum, 0, &n);
	pr_info("n = %d, slen=%ld\n", n, strlen(snum));
	/*
	slab_custom.c:60:  [1] (buffer) strlen:
  Does not handle strings that are not \0-terminated; if given one it may
  perform an over-read (it could cause a crash if unprotected) (CWE-126).
    */
#endif

	pr_debug("sizeof(MyStruct) = %lu\n", sizeof(struct MyStruct));
	/*
	 * struct kmem_cache *
	 * kmem_cache_create( const char *name, size_t size, size_t align,
	 * unsigned long flags,
	 * void (*ctor)(void*));
	 */
	my_cachep = kmem_cache_create(DRVNAME,	/* Name of slab cache */
				      sizeof(struct MyStruct), 0,	/* Alignment */
				      SLAB_HWCACHE_ALIGN,	/* Flags */
				      NULL);	/* Constructor */
}

static int slabsee_init(void)
{
	init_my_cache();
	mycache_use();
	pr_info("initialized\n");

	return 0;	/* success */
}

static void slabsee_exit(void)
{
	kmem_cache_destroy(my_cachep);
	pr_info("removed\n");
}

module_init(slabsee_init);
module_exit(slabsee_exit);

MODULE_LICENSE("MIT");
