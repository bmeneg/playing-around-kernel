/*
 * Copyright (c) 2018 Bruno E. O. Meneguele <bmeneguele@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "utils.h"

#define MYFS_MAGIC 0x4D594653

struct inode * myfs_create_inode(struct super_block *sb, umode_t mode)
{
	struct inode *inode;

	inode = new_inode(sb);
	if (inode) {
		inode->i_ino = get_next_ino();
		inode_init_owner(inode, NULL, mode);
		inode->i_atime = inode->i_mtime = inode->i_ctime =
			current_time(inode);
	} else {
		PR_ERROR("failed to create inode");
	}

	return inode;
}

int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *inode;

	sb->s_magic = MYFS_MAGIC;
	inode = myfs_create_inode(sb, S_IFDIR);
	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		return -ENOMEM;

	return 0;
}

struct dentry * myfs_mount(struct file_system_type *fs_type, int flags, const
			   char *dev_name, void *data)
{
	struct dentry *root_dentry;

	root_dentry = mount_bdev(fs_type, flags, dev_name, data,
				 myfs_fill_super);
	if (IS_ERR(root_dentry))
		PR_ERROR("failed to mount myfs. error %ld\n",
			 PTR_ERR(root_dentry));
	else
		PR_DEBUG("sucessfully mounted myfs\n");

	return root_dentry;
}

struct file_system_type myfs_type = {
	.owner = THIS_MODULE,
	.name = "myfs",
	.mount = myfs_mount,
	.fs_flags = FS_REQUIRES_DEV,
};
MODULE_ALIAS_FS("myfs");

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
