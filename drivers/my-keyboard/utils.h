/*
 * Copyright (c) 2017 Bruno E. O. Meneguele <bmeneguele@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 */

#ifndef __UTILS_H
#define __UTILS_H

#define __PR_FMT(log_lvl, fmt, ...) \
	printk(log_lvl "[%s] %s:%d:: " fmt, \
	       KBUILD_MODNAME, __func__, __LINE__, ##__VA_ARGS__)

#define PR_DEBUG(fmt, ...) \
	__PR_FMT(KERN_NOTICE, fmt, ##__VA_ARGS__)

#define PR_ERROR(fmt, ...) \
	__PR_FMT(KERN_ERR, fmt, ##__VA_ARGS__)

#endif
