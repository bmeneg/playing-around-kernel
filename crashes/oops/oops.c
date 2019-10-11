#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "utils.h"

static void create_oops(void)
{
	*(int *)0 = 0;
}

static int __init my_oops_init(void)
{
	PR_DEBUG("Hello world! Lets cause some mess!\n");
	create_oops();
	return 0;
}

static void __exit my_oops_exit(void)
{
	PR_DEBUG("Byee");
}

module_init(my_oops_init);
module_exit(my_oops_exit);

MODULE_AUTHOR("Bruno E. O. Meneguele <bmeneguele@gmail.com>");
MODULE_DESCRIPTION("Create Kernel Oops");
MODULE_LICENSE("GPL");
