/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CLOCK_PRIVATE_H
#define CLOCK_PRIVATE_H

#include <clock.h>
#include <device.h>
#include <stdbool.h>
#include <stdint.h>

#define CLOCK_DEVICE_STATE_INIT(n) \
	(struct device_state *) \
	&(char[sizeof_struct(struct clock_device_state, cs, n)]) { 0 }

#define CLOCK_PARENT(d, i) \
	& (const struct clock_handle) { \
		.dev = &(d).dev, \
		.id  = (i), \
	}

#define CLOCK_PARENTS(n) (const struct clock_handle[n])

#define FIXED_CLOCK(n, r) \
	{ \
		.info = { \
			.name     = (n), \
			.min_rate = (r), \
			.max_rate = (r), \
		}, \
	}

struct clock_state {
	uint8_t refcount;
};

struct clock_device_state {
	struct device_state ds;
	struct clock_state  cs[];
};

struct clock_driver_ops {
	struct clock_info *
	    (*get_info)(const struct clock_handle *clock);
	const struct clock_handle *
	    (*get_parent)(const struct clock_handle *clock);
	int (*get_rate)(const struct clock_handle *clock, uint32_t *rate);
	int (*get_state)(const struct clock_handle *clock, bool *state);
	int (*set_state)(const struct clock_handle *clock, bool enable);
};

struct clock_driver {
	const struct driver           drv;
	const struct clock_driver_ops ops;
};

#endif /* CLOCK_PRIVATE_H */
