/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <clock.h>
#include <device.h>
#include <stdint.h>
#include <clock/ccu.h>
#include <platform/devices.h>

#include "ccu.h"

#define APB2_CFG_REG    0x0058

#define APB2_CLK_SRC(x) ((x) << 24)
#define APB2_CLK_P(x)   ((x) << 16)
#define APB2_CLK_M(x)   ((x) << 0)

static uint32_t
sun8i_a83t_ccu_fixed_get_rate(const struct ccu *self UNUSED,
                              const struct ccu_clock *clk UNUSED,
                              uint32_t rate UNUSED)
{
	return 600000000U;
}

/*
 * APB2 has a mux, but it is assumed to always select OSC24M. Reparenting APB2
 * to PLL_PERIPH0 in Linux for faster UART clocks is unsupported.
 */
static const struct clock_handle sun8i_a83t_ccu_apb2_parent = {
	.dev = &r_ccu.dev,
	.id  = CLK_OSC24M,
};

static const struct clock_handle *
sun8i_a83t_ccu_apb2_get_parent(const struct ccu *self UNUSED,
                               const struct ccu_clock *clk UNUSED)
{
	return &sun8i_a83t_ccu_apb2_parent;
}

static const struct clock_handle sun8i_a83t_ccu_apb2_dev_parent = {
	.dev = &ccu.dev,
	.id  = CLK_APB2,
};

UNUSED static const struct clock_handle *
sun8i_a83t_ccu_apb2_dev_get_parent(const struct ccu *self UNUSED,
                                   const struct ccu_clock *clk UNUSED)
{
	return &sun8i_a83t_ccu_apb2_dev_parent;
}

static const struct ccu_clock sun8i_a83t_ccu_clocks[SUN8I_A83T_CCU_CLOCKS] = {
	[CLK_PLL_PERIPH0] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = sun8i_a83t_ccu_fixed_get_rate,
	},
	[CLK_APB2] = {
		.get_parent = sun8i_a83t_ccu_apb2_get_parent,
		.get_rate   = ccu_helper_get_rate,
	},
	[CLK_BUS_MSGBOX] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0064, 21),
		.reset      = BITMAP_INDEX(0x02c4, 21),
	},
	[CLK_BUS_PIO] = {
		.get_parent = ccu_helper_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x0068, 5),
	},
#if CONFIG(SERIAL_DEV_UART0)
	[CLK_BUS_UART0] = {
		.get_parent = sun8i_a83t_ccu_apb2_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x006c, 16),
		.reset      = BITMAP_INDEX(0x02d8, 16),
	},
#elif CONFIG(SERIAL_DEV_UART1)
	[CLK_BUS_UART1] = {
		.get_parent = sun8i_a83t_ccu_apb2_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x006c, 17),
		.reset      = BITMAP_INDEX(0x02d8, 17),
	},
#elif CONFIG(SERIAL_DEV_UART2)
	[CLK_BUS_UART2] = {
		.get_parent = sun8i_a83t_ccu_apb2_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x006c, 18),
		.reset      = BITMAP_INDEX(0x02d8, 18),
	},
#elif CONFIG(SERIAL_DEV_UART3)
	[CLK_BUS_UART3] = {
		.get_parent = sun8i_a83t_ccu_apb2_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x006c, 19),
		.reset      = BITMAP_INDEX(0x02d8, 19),
	},
#elif CONFIG(SERIAL_DEV_UART4)
	[CLK_BUS_UART4] = {
		.get_parent = sun8i_a83t_ccu_apb2_dev_get_parent,
		.get_rate   = ccu_helper_get_rate,
		.gate       = BITMAP_INDEX(0x006c, 20),
		.reset      = BITMAP_INDEX(0x02d8, 20),
	},
#endif
};

const struct ccu ccu = {
	.dev = {
		.name  = "ccu",
		.drv   = &ccu_driver.drv,
		.state = CLOCK_DEVICE_STATE_INIT(SUN8I_A83T_CCU_CLOCKS),
	},
	.clocks = sun8i_a83t_ccu_clocks,
	.regs   = DEV_CCU,
};

void
ccu_suspend(void)
{
}

void
ccu_resume(void)
{
}

void
ccu_init(void)
{
	/* Set APB2 to OSC24M/1 (24MHz). */
	mmio_write_32(DEV_CCU + APB2_CFG_REG,
	              APB2_CLK_SRC(1) |
	              APB2_CLK_P(0) |
	              APB2_CLK_M(0));

	ccu_resume();
}
