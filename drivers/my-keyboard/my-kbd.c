/*
 * Copyright (c) 2017 Bruno E. O. Meneguele <bmeneguele@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>

#include "utils.h"

#define KBD_IRQN 12

static struct input_dev *kbd_dev;
static volatile int irq_ref_count = 0;

static irqreturn_t kbd_irq_handler(int irq, void *dev)
{
	irq_ref_count++;
	return IRQ_HANDLED;
}

static int __init kbd_init(void)
{
	int err;

	kbd_dev = input_allocate_device();
	if (!kbd_dev) {
		PR_ERROR("not enough memory available\n");
		return -ENOMEM;
	}

	kbd_dev->name = "Bmeneg's Keyboard";
	set_bit(EV_KEY, kbd_dev->evbit);

	err = input_register_device(kbd_dev);
	if (err) {
		PR_ERROR("failed to register device\n");
		goto err_free_dev;
	}

	/* Keyboard IRQ number conflicts with i8042 interrupt controller,
	 * because of that the interrupt handler is over a shared line. */
	err = request_irq(KBD_IRQN, &kbd_irq_handler, IRQF_SHARED,
			  kbd_dev->name, kbd_dev);
	if (err) {
		PR_ERROR("failed to register IRQ %d\n", KBD_IRQN);
		goto err_unregister_dev;
	}

	return 0;

err_unregister_dev:
	input_unregister_device(kbd_dev);
err_free_dev:
	input_free_device(kbd_dev);
	return err;

}

static void __exit kbd_exit(void)
{
	free_irq(KBD_IRQN, kbd_dev);
	input_unregister_device(kbd_dev);
	PR_DEBUG("irq reference counter: %d\n", irq_ref_count);
}

module_init(kbd_init);
module_exit(kbd_exit);

MODULE_AUTHOR("Bruno E. O. Meneguele <bmeneguele@gmail.com>");
MODULE_DESCRIPTION("My own keyboard driver");
MODULE_LICENSE("GPL");
