/*
 * Copyright (c) 2018 Bruno E. O. Meneguele <bmeneguele@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "utils.h"

struct file_system_type myfs_type = {
	.owner = THIS_MODULE,
	.name = "myfs",
};


static int __init myfs_init(void)
{
	int err;

	PR_DEBUG("myfs init\n");

	err = register_filesystem(&myfs_type);
	if (err) 
		PR_ERROR("failed to register myfs. error %d\n", err);
	else
		PR_DEBUG("sucessfully registered myfs\n");

	return err;
}

static void __exit myfs_exit(void)
{
	int err;

	err = unregister_filesystem(&myfs_type);
	if (err) 
		PR_ERROR("failed to unregister myfs. error %d\n", err);
	else
		PR_DEBUG("sucessfully unregistered myfs\n");

	PR_DEBUG("myfs exit\n");
}

module_init(myfs_init);
module_exit(myfs_exit);

MODULE_AUTHOR("Bruno E. O. Meneguele");
MODULE_DESCRIPTION("my own filesystem, just for fun");
MODULE_LICENSE("GPL");
