# RCU mechanism (Read-Copy-Update)

The demo in here presented is a external module built to simulate a situation
RCU mechanism could be applied.

Basically this mechanism is a lockless way to synchronize read and write threads
over a common/shared data. The concept is really simple: before any update a
copy of the shared date is made and updated, any new reference to the data the
copy will be presented, until the last "old" reading thread is referecing the
"old" data (before the update), once the last old thread is finished (grace
period) the copy becomes the only data existent and the old one is safely freed.

PS1.: For updates we should understand as insertions and deletions, while reads
are..  well.. reads.
PS2.: RCU doesn't synchronize updaters threads, it's just a sync among readers
AND writers. Somthing like spinlocks or mutexes should be used to updaters.

# Demo description

The idea applied in this demo is really simple:

- inserting and reading threads are handled in userspace through sysfs interface
- deletion is handled by kernel systime interrupt
- the shared data is a linked list within kernel space of dog informations (dummy subject)
- use RCU to synchronize updaters and readers threads

Detailed info is present in the source code itself. Feel free to read and test
every thing. In case you find any error, also feel free to get in touch.

# Running the demo

First of all, install the kernel -development package of your distro: usually
they're named as `kernel-devel` or `kernel-dev`. In case you have compiled your
own kernel, that's ok, just forget this `download` step.

Follow the steps below:

```
$ git clone https://github.com/bmeneguele/playing-around-kernel
$ cd playing-around-kernel/sync/rcu
$ make
# insmod rcu-linked-list.ko
```

Take a look in the kernel's message log (`dmesg`) to confirm that the
module was correctly loaded:

```
$ dmesg | tail
...
[rcu_linked_list] rcu_linked_list_init:231:: module loaded
```

To remove the module just type:

```
# rmmod rcu-linked-list
$ dmesg | tail
...
[rcu_linked_list] rcu_linked_list_exit:252:: module unloaded
```

## Linked-list insertion

To add elements to the linked list in kernel space you can echo some value to a
file in sysfs. The value pattern is `breed,age,training_is_easy`, being `age` in
months and which has the following types: `string,int,bool`, although the bool
value can be anything from 0 to INT_MAX, but will be handled internally as a
boolean value (anything different from 0 is true).

```
# echo Golden,4,1 > /sys/rcu-linked-list/dog
```

## Linked-list reading

To read the linked list content (elements/nodes) just `cat` the sysfs file:

```
# cat /sys/rcu-linked-list/dog
```

## Linked-list deletion

The fist element of the linked-list is deleted every 5 seconds by a kernel timer
callback function, which runs everytime the timer triggers (systime
interruption), hence this is not exported to the user space through sysfs
interface, it's automatically performed by the module.

# Conclusion

All that said, you can create thousands of userspace processes updating and
reading the linked-list, and all will update/read a consistent state of the
shared data. RCU doesn't guarantee data existance or correctness, it just ensure
the a consistent state of the data to all threads.
