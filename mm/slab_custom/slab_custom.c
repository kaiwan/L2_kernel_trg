/*
 slab_custom.c

 Simple demo of creating a custom slab cache.
 Notice how the kernel reuses an existing cache if your cache size is a
 close-enough match!

 Load this module & look up /proc/slabinfo ..

 (c) Kaiwan NB
 GPL v2.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include "../../convenient.h"

#define DRVNAME "slab_custom"
#define MYBUFSZ 350         //-- using size '350' seems to give us a new private slab cache

static struct kmem_cache *my_cachep=NULL;

typedef struct {
	u32 a, b, c;
	s8 *cptr;
	u16 valid;
	u8 intbuf[MYBUFSZ];
} MyStruct;

//static 
void mycache_use(void)
{
	void *object;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39)
	printk("Cache name is %s\n",
		kmem_cache_name(my_cachep));
#else
	printk("[ker ver > 2.6.38 cache name deprecated...]\n");
#endif
	printk("Cache object size is %u\n",
		kmem_cache_size(my_cachep));

	object = kmem_cache_alloc (my_cachep, GFP_KERNEL);

	memset(object, 0xae, sizeof(MyStruct));
	print_hex_dump_bytes(" ", DUMP_PREFIX_ADDRESS, object, sizeof(MyStruct));

	if (object) {
		kmem_cache_free (my_cachep, object);
	}
}

static void init_my_cache(void)
{
	MSG("sizeof(MyStruct) = %lu\n", sizeof(MyStruct));
	/*
	struct kmem_cache *
	kmem_cache_create( const char *name, size_t size, size_t align,
	unsigned long flags,
	void (*ctor)(void*));
	*/
	my_cachep = kmem_cache_create(
		DRVNAME, /* Name of slab cache */
		sizeof(MyStruct),
		0, /* Alignment */
		SLAB_HWCACHE_ALIGN, /* Flags */
		NULL ); /* Constructor */
		return;
}

static int slabsee_init(void)
{
	QP;
	init_my_cache();
	mycache_use();
	return 0;
}

static void slabsee_exit(void)
{
	QP;
	if (my_cachep)
		kmem_cache_destroy(my_cachep);
}

module_init(slabsee_init);
module_exit(slabsee_exit);

MODULE_LICENSE("GPL");
