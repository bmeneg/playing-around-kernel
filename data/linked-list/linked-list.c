#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>

#include "utils.h"

/* Dummy structure to examplify the linked list */
struct dog {
	struct list_head list;
	char *breed;
	int age; /* in months */
	bool training_easy;
};

LIST_HEAD(dog_list);

static int __init linked_list_init(void)
{
	struct dog *my_dog;

	/* Structure initialization */
	my_dog = (struct dog *) kmalloc(sizeof(struct dog), GFP_KERNEL);
	my_dog->breed = "Golden Retriever";
	my_dog->age = 2;
	my_dog->training_easy = true;

	/*
	 * Add new node in the tail of the list (before head), this way the list
	 * behaves like a queue (FIFO) data structure.
	 */
	list_add_tail(&my_dog->list, &dog_list);

	PR_DEBUG("module loaded\n");
	return 0;
}

static void __exit linked_list_exit(void)
{
	struct dog *entry;

	/*
	 * The call to list_for_each_entry() returns the node structure after
	 * the head and follows to the *next structure, picking the nodes in a
	 * FIFO way. list_for_each_entry() calls the container_of() macro, which
	 * acts over the structure offset to its members, for instance, 'list'
	 * is the name of the 'struct dog' member that represents the linked
	 * list and &dog_list is actually the head of the linked list, thus
	 * container_of() will return the entire node (struct dog) that holds
	 * the 'list' member, by calculating the offset between the member and
	 * the top of the structure through the struct's definition.
	 */
	list_for_each_entry(entry, &dog_list, list) {
		PR_DEBUG("dog: breed='%s', age='%d', training_easy='%s'",
			 entry->breed, entry->age,
			 entry->training_easy ? "true" : "false");
	}

	PR_DEBUG("module unloaded\n");
}

module_init(linked_list_init);
module_exit(linked_list_exit);

MODULE_AUTHOR("Bruno E. O. Meneguele <bmeneguele@gmail.com>");
MODULE_DESCRIPTION("Simple linked list implementation");
MODULE_LICENSE("GPL");
