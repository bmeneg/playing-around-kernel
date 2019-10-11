# Filesystem on Linux Kernel

Before we actually start it's important to take a look how a filesystem
is abstracted to the userspace, thus we can better understand the
interfaces we have to fulfill to make it accessible to userspace.

## Virtual FileSystem (VFS)

VFS is the abstraction layer between the userspace and the actual
filesystem implementation on kernel. That's the interface that user has
to call when an access (read/write) is required to a file or directory.
The VSF make life easier when multiple filesystems types need to
exchange some information, like when a user want to send a file in an,
ie., usb stick using FAT32 filesystem, to a hard drive using ext4. Both
filesystems doens't know nothing about each other, it's VSF role to make
the exchange to work properly.

Because of this abstraction layer all filesystem must follow its
conventions (data structures, function signatures, and so on) and it's
the first thing we must check to start to implement a filesystem from
scratch.

## VFS architecture

VFS follows an object-oriented scheme to manage its data, in other
words, every data that moves on VFS is handled as objects (C
structures). There are four primary object types:

* **superblock**, which represents a specific mounted filesystem
* **inode**, which represents a specific file
* **dentry**, which represents a directory entry in a path
* **file**, which represents an open file as associated with a process

_note: VFS treats directories as normal files, so *dentry* isn't the
same as a directory, but directory is another kind of file._

Each object type (presented above) has an _operations_ object within,
that represents the operations the kernel can invoke against them. These
four primary object types are the basic ones, that every filesystem must
understand and have the _operations_ fulfilled as disered (per design),
but there are some other structures that each filesystem need to create
in order to make its place on kernel, much like any other subsystem's
objects around the kernel. These structures are the ones responsible to
announce the existence, registration, mapping, ... of the filesystem.
Lets take a closer look on them.

## Registring and Mouting a Filesystem

All structure headers and function signatures for this step are present
on _include/linux/fs.h_:

```
#include <linux/fs.h>

extern int register_filesystem(struct file_system_type *);
extern int unregister_filesystem(struct file_system_type *);
```

As you can see, it requires *struct file\_system\_type* strutucture, which
basically describes your and represents your filesystem. The structure
itself is declared as follows (referecing kernel 4.17):

```
struct file_system_type {
	const char *name;
	int fs_flags;
	struct dentry *(*mount) (struct file_system_type *, int,
		       const char *, void *);
	void (*kill_sb) (struct super_block *);
	struct module *owner;
	struct file_system_type * next;
	struct hlist_head fs_supers;

	struct lock_class_key s_lock_key;
	struct lock_class_key s_umount_key;
	struct lock_class_key s_vfs_rename_key;
	struct lock_class_key s_writers_key[SB_FREEZE_LEVELS];

	struct lock_class_key i_lock_key;
	struct lock_class_key i_mutex_key;
	struct lock_class_key i_mutex_dir_key;
};
```

Considering we are creating this filesystem from scratch I'll try to
explain each of this attributes when it's really necessary. But to start
with you basically need to fulfill _name_ and _owner_, which the first
is the name in _char_ format, i.e. "ext3", "ntfs", and the seconde should
be THIS_MODULE in most cases, which is basically used internally by
the subsystem to maintain the reference counter to the object (it isn't
used only on filesystems, but almost every place around the kernel),
preventing its delition in case the filesystem is being used somehow by
someone.

The second thing to be done is to implement the *mount* method, which will be
called by mount(2) system call from userspace applications. This method returns
the root *dentry* object that represents the root of that filesystem, being it
the part of another filesystem or not. For instance,
*/home/bmeneg/myfs_root/some_file*, if we mounted *myfs_root* with *myfs*
filesystem type then it will be the root of our filesystem returned by the
*mount* method present on *file_type_system* structure.

There are generic functions that can be used within *mount*, but a superblock
filler callback must be implemented. This *fill_super* is the responsible to
populate the superblock with informations about our filesystem.

```
int fill_super(struct super_block *sb, void *data, int silent)
```

The **sb** argument is the super block in-contruction and we're going to return
to it more then once during this talk, but just to start you basically need to
set the *magic number* for your filesystem, hence it can be distinguished from
other filesystems and also set the root *inode* that represents the entry
directory of your filesystem.

# References (TBD)
Linux Kernel Development book

VFS manual page on kernel source (Documentation/filesystem/vfs.txt)

https://lwn.net/Articles/13325/ (pretty old stuff, but has some insights)
