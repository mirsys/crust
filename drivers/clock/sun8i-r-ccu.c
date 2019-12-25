/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <clock.h>
#include <debug.h>
#include <device.h>
#include <error.h>
#include <stdint.h>
#include <clock/sunxi-ccu.h>
#include <platform/devices.h>

#include "sunxi-ccu.h"

static const uint32_t sun8i_r_ccu_fixed_rates[] = {
	[CLK_OSC16M] = 16000000U,
	[CLK_OSC24M] = 24000000U,
	[CLK_OSC32K] = 32768U,
};

static int
sun8i_r_ccu_fixed_rate(const struct sunxi_ccu *self UNUSED,
                       uint8_t id, uint32_t *rate)
{
	assert(id < ARRAY_SIZE(sun8i_r_ccu_fixed_rates));

	*rate = sun8i_r_ccu_fixed_rates[id];

	return SUCCESS;
}

static int
sun8i_r_ccu_ar100_rate(const struct sunxi_ccu *self,
                       uint8_t id, uint32_t *rate)
{
	const struct sunxi_ccu_clock *clk = &self->clocks[id];
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	/* Parent 2 (CLK_PLL_PERIPH0) has an additional divider. */
	if (bitfield_get(val, clk->mux) == 2)
		*rate /= bitfield_get(val, BITFIELD(8, 5)) + 1;

	return SUCCESS;
}

static struct sunxi_ccu_clock sun8i_r_ccu_clocks[SUN8I_R_CCU_CLOCKS] = {
	[CLK_OSC16M] = {
		.info.name = "osc16m",
		.get_rate  = sun8i_r_ccu_fixed_rate,
	},
	[CLK_OSC24M] = {
		.info.name = "osc24m",
		.get_rate  = sun8i_r_ccu_fixed_rate,
	},
	[CLK_OSC32K] = {
		.info.name = "osc32k",
		.get_rate  = sun8i_r_ccu_fixed_rate,
	},
	[CLK_AR100] = {
		.info = {
			.name     = "ar100",
			.max_rate = 300000000,
		},
		.parents = CLOCK_PARENTS(4) {
			{ .dev = &r_ccu.dev, .id = CLK_OSC32K },
			{ .dev = &r_ccu.dev, .id = CLK_OSC24M },
			{ .dev = &ccu.dev, .id = CLK_PLL_PERIPH0 },
			{ .dev = &r_ccu.dev, .id = CLK_OSC16M },
		},
		.get_rate = sun8i_r_ccu_ar100_rate,
		.reg      = 0x0000,
		.mux      = BITFIELD(16, 2),
		.p        = BITFIELD(4, 2),
	},
	[CLK_AHB0] = {
		.info.name = "ahb0",
		.parents   = CLOCK_PARENT(r_ccu, CLK_AR100),
	},
	[CLK_APB0] = {
		.info.name = "apb0",
		.parents   = CLOCK_PARENT(r_ccu, CLK_AHB0),
		.reg       = 0x000c,
		.p         = BITFIELD(0, 2),
	},
	[CLK_BUS_R_PIO] = {
		.info.name = "r_pio",
		.parents   = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate      = BITMAP_INDEX(0x0028 >> 2, 0),
	},
	[CLK_BUS_R_CIR] = {
		.info = {
			.name     = "r_cir",
			.max_rate = 100000000,
		},
		.parents = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate    = BITMAP_INDEX(0x0028 >> 2, 1),
		.reset   = BITMAP_INDEX(0x00b0 >> 2, 1),
	},
	[CLK_BUS_R_TIMER] = {
		.info.name = "r_timer",
		.parents   = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate      = BITMAP_INDEX(0x0028 >> 2, 2),
		.reset     = BITMAP_INDEX(0x00b0 >> 2, 2),
	},
	[CLK_BUS_R_RSB] = {
		.info.name = "r_rsb",
		.parents   = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate      = BITMAP_INDEX(0x0028 >> 2, 3),
		.reset     = BITMAP_INDEX(0x00b0 >> 2, 3),
	},
	[CLK_BUS_R_UART] = {
		.info.name = "r_uart",
		.parents   = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate      = BITMAP_INDEX(0x0028 >> 2, 4),
		.reset     = BITMAP_INDEX(0x00b0 >> 2, 4),
	},
	[CLK_BUS_R_I2C] = {
		.info.name = "r_i2c",
		.parents   = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate      = BITMAP_INDEX(0x0028 >> 2, 6),
		.reset     = BITMAP_INDEX(0x00b0 >> 2, 6),
	},
	[CLK_BUS_R_TWD] = {
		.info.name = "r_twd",
		.parents   = CLOCK_PARENT(r_ccu, CLK_APB0),
		.gate      = BITMAP_INDEX(0x0028 >> 2, 7),
	},
	[CLK_R_CIR] = {
		.info.name = "r_cir_mod",
		.parents   = CLOCK_PARENTS(4) {
			{ .dev = &r_ccu.dev, .id = CLK_OSC32K },
			{ .dev = &r_ccu.dev, .id = CLK_OSC24M },
		},
		.gate = BITMAP_INDEX(0x0054 >> 2, 31),
		.reg  = 0x0054,
		.mux  = BITFIELD(24, 2),
		.m    = BITFIELD(0, 4),
		.p    = BITFIELD(16, 2),
	},
};

const struct sunxi_ccu r_ccu = {
	.dev = {
		.name  = "r_ccu",
		.drv   = &sunxi_ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN8I_R_CCU_CLOCKS),
	},
	.clocks = sun8i_r_ccu_clocks,
	.regs   = DEV_R_PRCM,
};