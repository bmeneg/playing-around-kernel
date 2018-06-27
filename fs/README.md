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
on _include/linux/fs.h_.

# References (TBD)
Linux Kernel Development book

VFS manual page on kernel source (Documentation/filesystem/vfs.txt)
