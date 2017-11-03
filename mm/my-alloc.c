#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/page_ref.h>

#define __PR_FMT(log_level, fmt, ...) \
	printk(log_level "[%s] %s:%d:: " fmt, \
	       KBUILD_MODNAME , __func__, __LINE__, ##__VA_ARGS__)

#define PR_DEBUG(fmt, ...) \
	__PR_FMT(KERN_NOTICE, fmt, ##__VA_ARGS__)

#define PR_ERROR(fmt, ...) \
	__PR_FMT(KERN_ERR, fmt, ##__VA_ARGS__)


struct test {
	/* Kernel has its own defined types, for example these used in this
	 * structure. If certain code is architecture sensible it's a good idea
	 * define the type explicitly, i.e. u64, u32, ..., but when code is
	 * generic the default C type might be used. */
	u64 first;
	u32 second;
};

static int __init my_module_init(void)
{
	struct test *lets_go, *lets_stop;
	struct page *any_page;

	PR_DEBUG("hello world!\n");

	/* kmalloc allocates contiguous address directly in physical memory. To
	 * allocate virtually contiguous memory vmalloc should be used. But due
	 * to the amount of performance lost with vmalloc, kernel code tend to
	 * use kmalloc more often, although there are moments that vmalloc are
	 * necessary, i.e. when user requests memory allocation. */
	lets_go = kmalloc(sizeof(struct test), GFP_KERNEL);

	/* In case of error kmalloc returns NULL, because of that is mandatory
	 * check its return value. */
	if (!lets_go) {
		PR_ERROR("allocation to allowed\n");
		goto err;
	}

	lets_go->first = 1;
	lets_go->second = 2;
	PR_DEBUG("%p\n", lets_go);
	PR_DEBUG("%lld, %d\n", lets_go->first, lets_go->second);
	any_page = virt_to_page(lets_go);
	PR_DEBUG("page ref count: %d\n", page_ref_count(any_page));
	lets_stop = lets_go;
	PR_DEBUG("page ref count: %d\n", page_ref_count(any_page));

	kfree(lets_go);
	return 0;
err:
	return 1;
}

static void __exit my_module_exit(void)
{
	PR_DEBUG("bye world!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_AUTHOR("Bruno E. O. Meneguele <bmeneguele@gmail.com>");
MODULE_DESCRIPTION("Module to start my kernel studies");
MODULE_LICENSE("GPL");
