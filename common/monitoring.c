/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <debug.h>
#include <dm.h>
#include <dvfs.h>
#include <error.h>
#include <monitoring.h>
#include <sensor.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <system_power.h>
#include <dvfs/cpux.h>

#define HYSTERESIS_TEMP 5000
#define HYSTERESIS_TIME 5

#define TEMP_WARM       80000
#define TEMP_HOT        90000
#define TEMP_CRITICAL   100000

static uint8_t original_opp;
static uint8_t throttle_level;
static uint8_t time_cool;
static uint8_t time_warm;

static void
set_opp(void)
{
#if CONFIG_DVFS
	int err;

	/* Update the CPU's OPP. */
	if ((err = dvfs_set_opp(&cpux, 0, original_opp - throttle_level)))
		error("Failed to throttle CPU: %d", err);
#endif

	/* The throttle has changed. Reset the hysteresis timers. */
	time_cool = 0;
	time_warm = 0;
}

static void
start_throttling(bool immediate)
{
#if CONFIG_DVFS
	/* Record the OPP in use before throttling. */
	if (throttle_level == 0)
		original_opp = dvfs_get_opp(&cpux, 0);
#endif

	/* Do nothing if the system is fully throttled already. */
	if (throttle_level == original_opp || original_opp == 0)
		return;

	throttle_level = immediate ? original_opp : throttle_level + 1;
	set_opp();
}

static void
stop_throttling(void)
{
	/* Do nothing if the system is not throttled at all. */
	if (throttle_level == 0 || original_opp == 0)
		return;

	throttle_level -= 1;
	set_opp();
}

void
poll_sensors(void)
{
	struct device_handle sensor = DEVICE_HANDLE_INIT(DM_CLASS_SENSOR);
	uint32_t max_temp = 0, temp;

	while (dm_next_subdev(&sensor) == SUCCESS) {
		if (sensor_get_value(sensor.dev, sensor.id, &temp) != SUCCESS)
			continue;
		if (temp > max_temp)
			max_temp = temp;
	}

	if (max_temp >= TEMP_CRITICAL) {
		/* System is critically hot. Immediately shut down. */
		warn("System temperature over limit; shutting down");
		system_shutdown();
	} else if (max_temp >= TEMP_HOT) {
		/* System is hot. Immediately throttle to the minimum OPP. */
		if (throttle_level == 0)
			warn("System is hot; throttling CPU");
		start_throttling(true);
	} else if (max_temp >= TEMP_WARM) {
		/* System is warm. Throttle if it stays warm for a while. */
		if (++time_warm >= HYSTERESIS_TIME)
			start_throttling(false);
	} else if (max_temp < TEMP_WARM - HYSTERESIS_TEMP) {
		/* System is a bit below the warm limit. Reduce throttling. */
		if (++time_cool >= HYSTERESIS_TIME)
			stop_throttling();
	}
}

bool __pure
system_is_throttled(void)
{
	return throttle_level > 0;
}
