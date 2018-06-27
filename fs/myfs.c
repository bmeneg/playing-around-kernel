#include <linux/module.h>
#include <linux/kernel.h>

#include "utils.h"

static int __init myfs_init(void)
{
	PR_DEBUG("myfs init\n");
	return 0;
}

static void __exit myfs_exit(void)
{
	PR_DEBUG("myfs exit\n");
}

module_init(myfs_init);
module_exit(myfs_exit);

MODULE_AUTHOR("Bruno E. O. Meneguele");
MODULE_DESCRIPTION("my own filesystem, just for fun");
MODULE_LICENSE("GPL");
