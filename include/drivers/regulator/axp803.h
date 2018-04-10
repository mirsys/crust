/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_REGULATOR_AXP803_H
#define DRIVERS_REGULATOR_AXP803_H

#include <regulator.h>

#define AXP803_DRVDATA \
	(uintptr_t)(uint16_t[AXP803_REGL_COUNT])

enum {
	AXP803_REGL_DCDC1,
	AXP803_REGL_DCDC2,
	AXP803_REGL_DCDC3,
	AXP803_REGL_DCDC4,
	AXP803_REGL_DCDC5,
	AXP803_REGL_DCDC6,
	AXP803_REGL_DC1SW,
	AXP803_REGL_ALDO1,
	AXP803_REGL_ALDO2,
	AXP803_REGL_ALDO3,
	AXP803_REGL_DLDO1,
	AXP803_REGL_DLDO2,
	AXP803_REGL_DLDO3,
	AXP803_REGL_DLDO4,
	AXP803_REGL_ELDO1,
	AXP803_REGL_ELDO2,
	AXP803_REGL_ELDO3,
	AXP803_REGL_FLDO1,
	AXP803_REGL_FLDO2,
	AXP803_REGL_GPIO0,
	AXP803_REGL_GPIO1,
	AXP803_REGL_COUNT,
};

extern const struct regulator_driver axp803_regulator_driver;

#endif /* DRIVERS_REGULATOR_AXP803_H */
