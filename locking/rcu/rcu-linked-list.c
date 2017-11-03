#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rculist.h>
#include <linux/slab.h>

#include "utils.h"

struct dog {
	struct list_head list;
	char *breed;
	int age; /* in months */
	bool training_easy;
};

LIST_HEAD(dog_list);

static int __init rcu_linked_list_init(void)
{
	struct dog *my_dog;

	my_dog = (struct dog *) kmalloc(sizeof(struct dog), GFP_KERNEL);
	my_dog->breed = "Golden Retriever";
	my_dog->age = 2;
	my_dog->training_easy = true;
	list_add_tail_rcu(&my_dog->list, &dog_list);

	PR_DEBUG("module loaded\n");
	return 0;
}

static void __exit rcu_linked_list_exit(void)
{
	struct dog *entry;

	list_for_each_entry_rcu(entry, &dog_list, list) {
		PR_DEBUG("dog: breed='%s', age='%d', training_easy='%s'",
			 entry->breed, entry->age,
			 entry->training_easy ? "true" : "false");
	}

	PR_DEBUG("module unloaded\n");
}

module_init(rcu_linked_list_init);
module_exit(rcu_linked_list_exit);

MODULE_AUTHOR("Bruno E. O. Meneguele <bmeneguele@gmail.com>");
MODULE_DESCRIPTION("RCU mechanism over linked lists");
MODULE_LICENSE("GPL");
