#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define __PR_FMT(log_lvl, fmt, ...) \
	printk(log_lvl "[%s] %s:%d:: " fmt, \
	       KBUILD_MODNAME, __func__, __LINE__, ##__VA_ARGS__)

#define PR_DEBUG(fmt, ...) \
	__PR_FMT(KERN_NOTICE, fmt, ##__VA_ARGS__)

#define PR_ERROR(fmt, ...) \
	__PR_FMT(KERN_ERR, fmt, ##__VA_ARGS__)


static int __init hello_init(void)
{
	PR_DEBUG("Hello World\n");
	return 0;
}

static void __exit hello_exit(void)
{
	PR_DEBUG("Bye World\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Bruno E. O. Meneguele <bmeneguele@gmail.com>");
MODULE_DESCRIPTION("My second hello world module");
MODULE_LICENSE("GPL");
